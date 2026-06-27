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
///   var doc = RmlUISystem.Renderer.LoadDocument("/ui/main.rml");
///   // 每帧调用
///   RmlUISystem.Renderer.Update(deltaTime);
///   // 渲染由 ViewportSurface 处理
///   ViewportSurface_RenderRml(surface, RmlUISystem.Renderer.Handle);
///   // 清理
///   RmlUISystem.Shutdown();
/// </summary>
public static class RmlUISystem
{
    /// <summary>渲染器实例。</summary>
    private static RmlRenderer? _renderer;

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
    /// 关闭 RmlUI 系统。
    /// </summary>
    public static void Shutdown()
    {
        if (_renderer == null) return;

        _renderer.Dispose();
        _renderer = null;
        Console.WriteLine("[RmlUISystem] Shut down");
    }

    #region 便捷方法

    /// <summary>
    /// 加载文档。
    /// </summary>
    public static RmlDocument? LoadDocument(string path)
    {
        return _renderer?.LoadDocument(path);
    }

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
    public static bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers)
    {
        return _renderer?.OnKey(key, down, modifiers) ?? false;
    }

    /// <summary>
    /// 处理文本输入。
    /// </summary>
    public static bool OnTextInput(string text)
    {
        return _renderer?.OnTextInput(text) ?? false;
    }

    /// <summary>
    /// 每帧更新。
    /// </summary>
    public static void Update(float deltaTime)
    {
        _renderer?.Update(deltaTime);
    }

    /// <summary>
    /// 加载字体。
    /// </summary>
    public static bool LoadFontFace(string path, bool fallback = false)
    {
        return _renderer?.LoadFontFace(path, fallback) ?? false;
    }

    /// <summary>
    /// 重新加载所有文档。
    /// </summary>
    public static void ReloadAllDocuments()
    {
        _renderer?.ReloadAllDocuments();
    }

    /// <summary>
    /// 设置调试器可见性。
    /// </summary>
    public static void SetDebuggerVisible(bool visible)
    {
        _renderer?.SetDebuggerVisible(visible);
    }

    #endregion
}
