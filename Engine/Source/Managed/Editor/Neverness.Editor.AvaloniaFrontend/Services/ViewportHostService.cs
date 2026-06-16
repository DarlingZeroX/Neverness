using Neverness.Editor.AvaloniaFrontend.Viewport;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// 视口宿主服务——管理视口表面的生命周期。
///
/// 职责：
/// - 创建和销毁视口表面
/// - 管理原生窗口句柄
/// - 将句柄传递给 Diligent 渲染引擎
/// - 处理窗口 resize
/// </summary>
public class ViewportHostService : IDisposable
{
    private NativeControlHostSurface? _surface;
    private bool _disposed;

    /// <summary>当前视口表面。</summary>
    public IViewportSurface? Surface => _surface;

    /// <summary>是否有有效的表面。</summary>
    public bool HasSurface => _surface?.IsValid == true;

    /// <summary>
    /// 创建视口表面。
    /// </summary>
    public NativeControlHostSurface CreateSurface(int width, int height)
    {
        // 销毁旧表面
        _surface?.Dispose();

        // 创建新表面
        _surface = new NativeControlHostSurface(width, height);

        Console.WriteLine($"[ViewportHostService] 创建视口表面: {width}x{height}");

        return _surface;
    }

    /// <summary>
    /// 获取 ViewportHostControl 控件（需要添加到 Avalonia 可视树）。
    /// </summary>
    public ViewportHostControl? GetControl()
    {
        return _surface?.GetControl();
    }

    /// <summary>
    /// 获取原生窗口句柄（传递给 Diligent 渲染引擎）。
    /// </summary>
    public IntPtr GetNativeHandle()
    {
        return _surface?.GetNativeHandle() ?? IntPtr.Zero;
    }

    /// <summary>
    /// 获取原生句柄描述符（"HWND"/"X11"/"NSView"）。
    /// </summary>
    public string GetHandleDescriptor()
    {
        return _surface?.GetHandleDescriptor() ?? "";
    }

    /// <summary>
    /// 更新视口尺寸（窗口 resize 时调用）。
    /// </summary>
    public void Resize(int width, int height)
    {
        _surface?.Resize(width, height);
    }

    /// <summary>
    /// 销毁视口表面。
    /// </summary>
    public void DestroySurface()
    {
        if (_surface != null)
        {
            _surface.Dispose();
            _surface = null;
            Console.WriteLine("[ViewportHostService] 视口表面已销毁");
        }
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        DestroySurface();
    }
}
