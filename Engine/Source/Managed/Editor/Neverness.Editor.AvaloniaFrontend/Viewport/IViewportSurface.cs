namespace Neverness.Editor.AvaloniaFrontend.Viewport;

/// <summary>
/// 视口表面抽象接口——平台无关的渲染目标。
///
/// 设计原则：
/// - ViewModel 不持有 NativeWindowHandle（平台无关）
/// - View 通过此接口获取渲染目标
/// - 不同平台（Win32/X11/NSView）各自实现
///
/// 使用方式：
///   var surface = viewportHostService.CreateSurface(width, height);
///   var hwnd = surface.GetNativeHandle();
///   // 传递 hwnd 给 Diligent 渲染
/// </summary>
public interface IViewportSurface : IDisposable
{
    /// <summary>表面宽度。</summary>
    int Width { get; }

    /// <summary>表面高度。</summary>
    int Height { get; }

    /// <summary>获取原生窗口句柄（HWND/X11 Window/NSView）。</summary>
    IntPtr GetNativeHandle();

    /// <summary>调整表面尺寸。</summary>
    void Resize(int width, int height);

    /// <summary>表面是否有效。</summary>
    bool IsValid { get; }

    /// <summary>表面创建事件。</summary>
    event Action<IntPtr>? SurfaceCreated;

    /// <summary>表面销毁事件。</summary>
    event Action? SurfaceDestroyed;

    /// <summary>表面尺寸变更事件。</summary>
    event Action<int, int>? SurfaceResized;
}
