using RmlUiNet;
using RmlUiNet.Input;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Rmlui.Internal;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 渲染器封装。
///
/// 职责：
/// - 持有 C++ RmlUIRenderer 的 Handle（用于 ViewportSurface 渲染）
/// - 管理 RmlUi.Net Context 生命周期
/// - 提供输入处理、更新、字体、调试等功能
///
/// 文档管理由 RmlDocumentManager 负责。
/// </summary>
public sealed class RmlRenderer : IDisposable
{
    /// <summary>C++ 渲染器 Handle。</summary>
    private uint _handle;

    /// <summary>RmlUi.Net Context，用于逻辑管理。</summary>
    private Context? _context;

    /// <summary>文档管理器。</summary>
    private RmlDocumentManager _documentManager;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>是否拥有全局初始化（由 Manager 创建时为 false）。</summary>
    private readonly bool _ownsGlobalInit;

    /// <summary>
    /// 创建渲染器（公开构造函数，自行管理全局初始化）。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    public RmlRenderer(int width, int height) : this(width, height, skipGlobalInit: false)
    {
    }

    /// <summary>
    /// 创建渲染器（内部构造函数，允许跳过全局初始化）。
    ///
    /// RmlRendererManager 创建多个渲染器时，只需第一个调 Rml.Initialise()，
    /// 后续渲染器传 skipGlobalInit=true 跳过。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    /// <param name="skipGlobalInit">是否跳过 Rml.Initialise()。</param>
    internal RmlRenderer(int width, int height, bool skipGlobalInit)
    {
        _ownsGlobalInit = !skipGlobalInit;

        // 创建 C++ 渲染器（初始化 Diligent 渲染后端 + RmlUI 全局接口）
        _handle = RmlNativeInterop.RmlRenderer_Create(width, height);
        if (_handle == 0)
            throw new InvalidOperationException("Failed to create RmlUI renderer");

        //if (_ownsGlobalInit)
        //{
        //    Rml.Initialise();
        //}

        // 创建 Context（RmlUi.Net 包装，每个渲染器独立 Context）
        _context = Rml.CreateContext($"RmlCtx_{_handle}", new Vector2i(width, height));
        if (_context == null)
            throw new InvalidOperationException("Failed to create RmlUI context");

        // 将 C# Context 的原生指针传给 C++ 渲染器，使两者共享同一个 Context
        RmlNativeInterop.RmlRenderer_SetContext(_handle, _context.NativePtr);

        // 创建文档管理器
        _documentManager = new RmlDocumentManager(_context);
    }

    #region 属性

    /// <summary>
    /// 渲染器 Handle（传给 ViewportSurface 执行渲染）。
    /// </summary>
    public uint Handle => _handle;

    /// <summary>
    /// RmlUi.Net Context。
    /// </summary>
    public Context? Context => _context;

    /// <summary>
    /// 文档管理器。
    /// </summary>
    public RmlDocumentManager DocumentManager => _documentManager;

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

    #region 场景同步

    /// <summary>
    /// 从场景读取 RmlUIDocumentComponent 并同步文档。
    ///
    /// 内部查询场景 ECS，通过 assetPathResolver 将 GUID 解析为 VFS 路径，
    /// 然后调用 DocumentManager.Sync 执行三路 Diff（新增加载、移除卸载、保留已有）。
    /// </summary>
    /// <param name="scene">场景实例（IScene 接口）。</param>
    /// <param name="assetPathResolver">GUID → VFS 路径解析器。为 null 时跳过同步。</param>
    public void SyncFromScene(IScene scene, Func<NNGuid, string?>? assetPathResolver)
    {
        if (assetPathResolver == null) return;

        var view = scene.CreateView<RmlUIDocumentComponent>();
        view.Refresh();

        var syncItems = new List<(ulong Entity, string VfsPath)>();

        view.ForEach((IEntity entity, RmlUIDocumentComponent doc) =>
        {
            var path = assetPathResolver(doc.DocumentAsset);
            if (!string.IsNullOrEmpty(path))
            {
                syncItems.Add(((ulong)entity.Id, path));
            }
        });

        _documentManager.Sync(syncItems);
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

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        // 释放文档管理器
        _documentManager.Clear();

        // 释放 Context
        _context?.Dispose();
        _context = null;

        // 销毁 C++ 渲染器
        if (_handle != 0)
        {
            RmlNativeInterop.RmlRenderer_Destroy(_handle);
            _handle = 0;
        }

        // 关闭 RmlUi（仅由拥有全局初始化的实例负责）
        if (_ownsGlobalInit)
        {
            Rml.Shutdown();
        }

        _disposed = true;
    }

    #endregion
}
