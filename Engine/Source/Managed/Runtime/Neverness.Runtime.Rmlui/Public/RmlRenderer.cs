using RmlUiNet;
using RmlUiNet.Input;
using Neverness.Runtime.Rmlui.Internal;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 渲染器封装。
///
/// 职责：
/// - 持有 C++ RmlUIRenderer 的 Handle（用于 ViewportSurface 渲染）
/// - 使用 RmlUi.Net 管理 Context、Document、输入、更新等逻辑
///
/// 架构：
/// - C++ 只管 GPU 渲染（RmlDiligentRenderInterface）
/// - C# 管理所有逻辑（文档、输入、更新）
/// - 渲染时把 Handle 传给 ViewportSurface
/// </summary>
public sealed class RmlRenderer : IDisposable
{
    /// <summary>C++ 渲染器 Handle。</summary>
    private uint _handle;

    /// <summary>RmlUi.Net Context，用于逻辑管理。</summary>
    private Context? _context;

    /// <summary>已加载的文档列表。</summary>
    private readonly List<RmlDocument> _documents = new();

    /// <summary>热重载管理器。</summary>
    private RmlHotReloader? _hotReloader;

    /// <summary>同步管理器。</summary>
    private SyncManager? _syncManager;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>
    /// 创建渲染器。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    public RmlRenderer(int width, int height)
    {
        // 创建 C++ 渲染器（用于 ViewportSurface 渲染）
        _handle = RmlNativeInterop.RmlRenderer_Create(width, height);
        if (_handle == 0)
            throw new InvalidOperationException("Failed to create RmlUI renderer");

        // 初始化 RmlUi.Net（用于逻辑管理）
        Rml.Initialise();

        // 创建 Context（使用公共 API）
        _context = Rml.CreateContext("Main", new Vector2i(width, height));
        if (_context == null)
            throw new InvalidOperationException("Failed to create RmlUI context");
    }

    #region 属性

    /// <summary>
    /// 渲染器 Handle（传给 ViewportSurface 执行渲染）。
    /// </summary>
    public uint Handle => _handle;

    /// <summary>
    /// RmlUi.Net Context。
    /// </summary>
    internal Context? Context => _context;

    /// <summary>
    /// 已加载的文档列表（只读）。
    /// </summary>
    public IReadOnlyList<RmlDocument> Documents => _documents;

    /// <summary>
    /// 热重载管理器（懒加载）。
    /// </summary>
    public RmlHotReloader HotReloader
    {
        get
        {
            _hotReloader ??= new RmlHotReloader(this);
            return _hotReloader;
        }
    }

    /// <summary>
    /// 同步管理器（懒加载）。
    /// </summary>
    public SyncManager Sync
    {
        get
        {
            _syncManager ??= new SyncManager(this);
            return _syncManager;
        }
    }

    /// <summary>
    /// 调试器是否可见。
    /// </summary>
    public bool IsDebuggerVisible => RmlUiNet.Debugger.IsVisible();

    #endregion

    #region 视口

    /// <summary>
    /// 调整视口尺寸。
    /// </summary>
    public void Resize(int width, int height)
    {
        _context?.SetDimensions(width, height);
    }

    #endregion

    #region 文档管理

    /// <summary>
    /// 加载文档。
    /// </summary>
    /// <param name="path">文档 VFS 路径。</param>
    /// <param name="autoShow">是否自动显示。</param>
    /// <returns>文档实例，失败返回 null。</returns>
    public RmlDocument? LoadDocument(string path, bool autoShow = true)
    {
        if (_context == null) return null;

        var doc = _context.LoadDocument(path);
        if (doc == null) return null;

        var rmlDoc = new RmlDocument(this, doc, path);
        _documents.Add(rmlDoc);

        if (autoShow)
        {
            rmlDoc.Show();
        }

        return rmlDoc;
    }

    /// <summary>
    /// 获取指定路径的文档。
    /// </summary>
    /// <param name="path">文档路径。</param>
    /// <returns>文档实例，未找到返回 null。</returns>
    public RmlDocument? GetDocument(string path)
    {
        return _documents.FirstOrDefault(d =>
            string.Equals(d.Path, path, StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>
    /// 卸载文档（通过路径）。
    /// </summary>
    /// <param name="path">文档路径。</param>
    public void UnloadDocument(string path)
    {
        var doc = GetDocument(path);
        if (doc != null)
        {
            doc.Close();
            _documents.Remove(doc);
            _hotReloader?.UnregisterDocument(doc);
        }
    }

    /// <summary>
    /// 卸载文档（内部调用）。
    /// </summary>
    internal void RemoveDocument(RmlDocument doc)
    {
        _documents.Remove(doc);
        _hotReloader?.UnregisterDocument(doc);
    }

    /// <summary>
    /// 重新加载所有文档。
    /// </summary>
    public void ReloadAllDocuments()
    {
        foreach (var doc in _documents)
        {
            if (doc.IsValid)
            {
                doc.Reload();
            }
        }
    }

    #endregion

    #region 输入处理

    /// <summary>
    /// 处理鼠标移动。
    /// </summary>
    public bool OnMouseMove(int x, int y, KeyModifier modifiers)
    {
        return _context?.ProcessMouseMove(x, y, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标按键。
    /// </summary>
    public bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers)
    {
        if (down)
            return _context?.ProcessMouseButtonDown((int)button, modifiers) ?? false;
        else
            return _context?.ProcessMouseButtonUp((int)button, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标滚轮。
    /// </summary>
    public bool OnMouseWheel(float delta, KeyModifier modifiers)
    {
        return _context?.ProcessMouseWheel(new Vector2f(0, delta), modifiers) ?? false;
    }

    /// <summary>
    /// 处理键盘按键（带快捷键支持）。
    /// </summary>
    public bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f)
    {
        if (!down) return _context?.ProcessKeyUp(key, modifiers) ?? false;

        // 优先级快捷键（在 Context 处理前）
        bool propagate = RmlShell.ProcessKeyDownShortcuts(
            _context!, key, modifiers, nativeDpRatio, priority: true);

        if (!propagate) return true;

        // 提交给 Context 处理
        bool handled = _context?.ProcessKeyDown(key, modifiers) ?? false;

        if (!handled)
        {
            // 低优先级快捷键（在 Context 未处理时）
            propagate = RmlShell.ProcessKeyDownShortcuts(
                _context!, key, modifiers, nativeDpRatio, priority: false);
        }

        return handled || !propagate;
    }

    /// <summary>
    /// 处理文本输入。
    /// </summary>
    public bool OnTextInput(string text)
    {
        return _context?.ProcessTextInput(text) ?? false;
    }

    /// <summary>
    /// 处理鼠标离开窗口。
    /// </summary>
    public void OnMouseLeave()
    {
        _context?.ProcessMouseLeave();
    }

    #endregion

    #region 更新

    /// <summary>
    /// 每帧更新。
    /// </summary>
    public void Update(float deltaTime)
    {
        _context?.Update();
    }

    #endregion

    #region 字体管理

    /// <summary>
    /// 加载字体。
    /// </summary>
    public bool LoadFontFace(string path, bool fallback = false)
    {
        return Rml.LoadFontFace(path, fallback);
    }

    /// <summary>
    /// 加载默认字体集。
    /// </summary>
    public void LoadDefaultFonts(string fontsPath)
    {
        RmlShell.LoadFonts(fontsPath);
    }

    #endregion

    #region 调试

    /// <summary>
    /// 设置调试器可见性。
    /// </summary>
    public void SetDebuggerVisible(bool visible)
    {
        RmlUiNet.Debugger.SetVisible(visible);
    }

    /// <summary>
    /// 切换调试器可见性。
    /// </summary>
    public void ToggleDebugger()
    {
        SetDebuggerVisible(!IsDebuggerVisible);
    }

    #endregion

    #region SyncManager（内部类）

    /// <summary>
    /// 文档同步管理器。
    ///
    /// 对应 C++ RmlUIRenderer::Sync() 的三路 Diff 逻辑：
    /// - 删除不在列表中的文档
    /// - 加载新文档
    /// - 保持已存在的文档
    /// </summary>
    public sealed class SyncManager
    {
        private readonly RmlRenderer _renderer;
        private readonly HashSet<string> _activePaths = new(StringComparer.OrdinalIgnoreCase);

        internal SyncManager(RmlRenderer renderer)
        {
            _renderer = renderer;
        }

        /// <summary>
        /// 当前活跃的文档路径。
        /// </summary>
        public IReadOnlyCollection<string> ActivePaths => _activePaths;

        /// <summary>
        /// 同步文档列表（三路 Diff）。
        /// </summary>
        /// <param name="paths">要同步的文档路径列表。</param>
        public void Sync(IEnumerable<string> paths)
        {
            var newPaths = new HashSet<string>(paths, StringComparer.OrdinalIgnoreCase);

            // 1. 删除不在新列表中的文档
            var toRemove = new List<string>();
            foreach (var activePath in _activePaths)
            {
                if (!newPaths.Contains(activePath))
                {
                    toRemove.Add(activePath);
                }
            }

            foreach (var path in toRemove)
            {
                _renderer.UnloadDocument(path);
                _activePaths.Remove(path);
            }

            // 2. 加载新文档
            foreach (var path in newPaths)
            {
                if (_activePaths.Contains(path))
                    continue;

                var doc = _renderer.LoadDocument(path);
                if (doc != null)
                {
                    _activePaths.Add(path);
                }
            }
        }

        /// <summary>
        /// 添加单个文档。
        /// </summary>
        public bool Add(string path)
        {
            if (_activePaths.Contains(path))
                return true;

            var doc = _renderer.LoadDocument(path);
            if (doc != null)
            {
                _activePaths.Add(path);
                return true;
            }

            return false;
        }

        /// <summary>
        /// 移除单个文档。
        /// </summary>
        public void Remove(string path)
        {
            if (!_activePaths.Contains(path))
                return;

            _renderer.UnloadDocument(path);
            _activePaths.Remove(path);
        }

        /// <summary>
        /// 清除所有文档。
        /// </summary>
        public void Clear()
        {
            foreach (var path in _activePaths)
            {
                _renderer.UnloadDocument(path);
            }
            _activePaths.Clear();
        }

        /// <summary>
        /// 重新加载所有活跃文档。
        /// </summary>
        public void ReloadAll()
        {
            foreach (var path in _activePaths)
            {
                var doc = _renderer.GetDocument(path);
                doc?.Reload();
            }
        }
    }

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        // 释放同步管理器
        _syncManager?.Clear();

        // 释放热重载管理器
        _hotReloader?.Dispose();
        _hotReloader = null;

        // 释放所有文档
        foreach (var doc in _documents.ToArray())
            doc.Dispose();
        _documents.Clear();

        // 释放 Context
        _context?.Dispose();
        _context = null;

        // 销毁 C++ 渲染器
        if (_handle != 0)
        {
            RmlNativeInterop.RmlRenderer_Destroy(_handle);
            _handle = 0;
        }

        // 关闭 RmlUi
        Rml.Shutdown();

        _disposed = true;
    }

    #endregion
}
