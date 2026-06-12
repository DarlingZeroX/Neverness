using Avalonia.Controls;
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
///
/// 设计原则：
/// - NativeWindowHandle 不进 ViewModel
/// - 通过 IViewportSurface 抽象平台差异
/// - ViewportView 通过此服务获取渲染目标
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
    /// <param name="width">初始宽度。</param>
    /// <param name="height">初始高度。</param>
    /// <returns>创建的表面。</returns>
    public IViewportSurface CreateSurface(int width, int height)
    {
        // 销毁旧表面
        _surface?.Dispose();

        // 创建新表面
        _surface = new NativeControlHostSurface(width, height);

        Console.WriteLine($"[ViewportHostService] 创建视口表面: {width}x{height}");

        return _surface;
    }

    /// <summary>
    /// 获取 NativeControlHost 控件（需要添加到 Avalonia 可视树）。
    /// </summary>
    public NativeControlHost? GetControl()
    {
        return _surface?.GetControl();
    }

    /// <summary>
    /// 初始化原生句柄（在控件附加到可视树后调用）。
    /// </summary>
    public void InitializeNativeHandle()
    {
        _surface?.InitializeNativeHandle();
    }

    /// <summary>
    /// 获取原生窗口句柄（传递给 Diligent 渲染引擎）。
    /// </summary>
    /// <returns>原生窗口句柄，无效时返回 IntPtr.Zero。</returns>
    public IntPtr GetNativeHandle()
    {
        return _surface?.GetNativeHandle() ?? IntPtr.Zero;
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
