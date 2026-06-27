using RmlUiNet.Input;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 全局系统。
///
/// 提供便捷的静态方法访问 RmlRenderer。
/// 文档管理通过 DocumentManager 进行。
///
/// 使用方法：
///   RmlUISystem.Initialize(1920, 1080);
///   RmlUISystem.DocumentManager.LoadDocument(1, "/ui/main.rml");
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
    /// 获取 RmlUi Context。
    /// </summary>
    public static RmlUiNet.Context? Context => _renderer?.Context;

    /// <summary>
    /// 获取文档管理器。
    /// </summary>
    public static RmlDocumentManager? DocumentManager => _renderer?.DocumentManager;

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

    #region 文档管理委托

    /// <summary>
    /// 同步文档列表（三路 Diff）。
    /// </summary>
    public static void SyncDocuments(IEnumerable<(ulong Entity, string VfsPath)> items)
    {
        _renderer?.DocumentManager.Sync(items);
    }

    /// <summary>
    /// 加载单个文档。
    /// </summary>
    public static bool LoadDocument(ulong entity, string vfsPath)
    {
        return _renderer?.DocumentManager.LoadDocument(entity, vfsPath) ?? false;
    }

    /// <summary>
    /// 卸载单个文档。
    /// </summary>
    public static void UnloadDocument(ulong entity)
    {
        _renderer?.DocumentManager.UnloadDocument(entity);
    }

    /// <summary>
    /// 文件变化通知（热重载）。
    /// </summary>
    public static void NotifyFileChanged(string filePath)
    {
        _renderer?.DocumentManager.NotifyFileChanged(filePath);
    }

    #endregion
}
