using RmlUiNet.Input;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 全局系统。
///
/// 提供便捷的静态方法访问 RmlRenderer。
/// 管理渲染器生命周期和全局状态。
///
/// 使用方法：
///   RmlUISystem.Initialize(1920, 1080);
///   var doc = RmlUISystem.LoadDocument("/ui/main.rml");
///   // 每帧调用
///   RmlUISystem.Update(deltaTime);
///   // 渲染由 ViewportSurface 处理
///   ViewportSurface_RenderRml(surface, RmlUISystem.RendererHandle);
///   // 清理
///   RmlUISystem.Shutdown();
/// </summary>
public static class RmlUISystem
{
    /// <summary>渲染器实例。</summary>
    private static RmlRenderer? _renderer;

    #region 初始化/关闭

    /// <summary>
    /// 初始化 RmlUI 系统。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    public static void Initialize(int width, int height)
    {
        if (_renderer != null)
        {
            Console.WriteLine("[RmlUISystem] Already initialized, shutting down first...");
            Shutdown();
        }

        _renderer = new RmlRenderer(width, height);
        Console.WriteLine($"[RmlUISystem] Initialized with viewport {width}x{height}");
    }

    /// <summary>
    /// 关闭 RmlUI 系统。
    /// </summary>
    public static void Shutdown()
    {
        if (_renderer == null) return;

        _renderer.Dispose();
        _renderer = null;
        Console.WriteLine("[RmlUISystem] Shut down");
    }

    #endregion

    #region 属性

    /// <summary>
    /// 获取渲染器实例。
    /// </summary>
    public static RmlRenderer? Renderer => _renderer;

    /// <summary>
    /// 系统是否已初始化。
    /// </summary>
    public static bool IsInitialized => _renderer != null;

    /// <summary>
    /// 获取渲染器 Handle（用于 ViewportSurface）。
    /// </summary>
    public static uint RendererHandle => _renderer?.Handle ?? 0;

    /// <summary>
    /// 获取热重载管理器。
    /// </summary>
    public static RmlHotReloader? HotReloader => _renderer?.HotReloader;

    /// <summary>
    /// 获取同步管理器。
    /// </summary>
    public static RmlRenderer.SyncManager? Sync => _renderer?.Sync;

    #endregion

    #region 文档管理

    /// <summary>
    /// 加载文档。
    /// </summary>
    public static RmlDocument? LoadDocument(string path, bool autoShow = true)
    {
        return _renderer?.LoadDocument(path, autoShow);
    }

    /// <summary>
    /// 获取指定路径的文档。
    /// </summary>
    public static RmlDocument? GetDocument(string path)
    {
        return _renderer?.GetDocument(path);
    }

    /// <summary>
    /// 卸载文档。
    /// </summary>
    public static void UnloadDocument(string path)
    {
        _renderer?.UnloadDocument(path);
    }

    /// <summary>
    /// 重新加载所有文档。
    /// </summary>
    public static void ReloadAllDocuments()
    {
        _renderer?.ReloadAllDocuments();
    }

    #endregion

    #region 输入处理

    /// <summary>
    /// 处理鼠标移动。
    /// </summary>
    public static bool OnMouseMove(int x, int y, KeyModifier modifiers)
    {
        return _renderer?.OnMouseMove(x, y, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标按键。
    /// </summary>
    public static bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers)
    {
        return _renderer?.OnMouseButton(button, down, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标滚轮。
    /// </summary>
    public static bool OnMouseWheel(float delta, KeyModifier modifiers)
    {
        return _renderer?.OnMouseWheel(delta, modifiers) ?? false;
    }

    /// <summary>
    /// 处理键盘按键。
    /// </summary>
    public static bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f)
    {
        return _renderer?.OnKey(key, down, modifiers, nativeDpRatio) ?? false;
    }

    /// <summary>
    /// 处理文本输入。
    /// </summary>
    public static bool OnTextInput(string text)
    {
        return _renderer?.OnTextInput(text) ?? false;
    }

    /// <summary>
    /// 处理鼠标离开窗口。
    /// </summary>
    public static void OnMouseLeave()
    {
        _renderer?.OnMouseLeave();
    }

    #endregion

    #region 更新

    /// <summary>
    /// 每帧更新。
    /// </summary>
    public static void Update(float deltaTime)
    {
        _renderer?.Update(deltaTime);
    }

    #endregion

    #region 字体管理

    /// <summary>
    /// 加载字体。
    /// </summary>
    public static bool LoadFontFace(string path, bool fallback = false)
    {
        return _renderer?.LoadFontFace(path, fallback) ?? false;
    }

    /// <summary>
    /// 加载默认字体集。
    /// </summary>
    public static void LoadDefaultFonts(string fontsPath)
    {
        _renderer?.LoadDefaultFonts(fontsPath);
    }

    #endregion

    #region 调试

    /// <summary>
    /// 设置调试器可见性。
    /// </summary>
    public static void SetDebuggerVisible(bool visible)
    {
        _renderer?.SetDebuggerVisible(visible);
    }

    /// <summary>
    /// 切换调试器可见性。
    /// </summary>
    public static void ToggleDebugger()
    {
        _renderer?.ToggleDebugger();
    }

    #endregion

    #region 热重载

    /// <summary>
    /// 文件变化通知。
    /// </summary>
    public static void NotifyFileChanged(string filePath)
    {
        _renderer?.HotReloader.NotifyFileChanged(filePath);
    }

    /// <summary>
    /// 注册文档到热重载。
    /// </summary>
    public static void RegisterDocumentForReload(RmlDocument document, IEnumerable<string>? watchedFiles = null)
    {
        _renderer?.HotReloader.RegisterDocument(document, watchedFiles);
    }

    #endregion

    #region 文档同步

    /// <summary>
    /// 同步文档列表（三路 Diff）。
    /// </summary>
    public static void SyncDocuments(IEnumerable<string> paths)
    {
        _renderer?.Sync.Sync(paths);
    }

    /// <summary>
    /// 添加单个文档到同步列表。
    /// </summary>
    public static bool SyncAddDocument(string path)
    {
        return _renderer?.Sync.Add(path) ?? false;
    }

    /// <summary>
    /// 从同步列表移除文档。
    /// </summary>
    public static void SyncRemoveDocument(string path)
    {
        _renderer?.Sync.Remove(path);
    }

    /// <summary>
    /// 清除同步列表中的所有文档。
    /// </summary>
    public static void SyncClearDocuments()
    {
        _renderer?.Sync.Clear();
    }

    /// <summary>
    /// 重新加载同步列表中的所有文档。
    /// </summary>
    public static void SyncReloadAllDocuments()
    {
        _renderer?.Sync.ReloadAll();
    }

    #endregion
}
