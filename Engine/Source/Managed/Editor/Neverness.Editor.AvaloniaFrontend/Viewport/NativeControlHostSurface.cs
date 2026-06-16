using Avalonia.Controls;

namespace Neverness.Editor.AvaloniaFrontend.Viewport;

/// <summary>
/// NativeControlHost 视口表面实现——使用 ViewportHostControl 嵌入原生窗口。
///
/// 工作原理：
/// 1. 创建 ViewportHostControl（子类化 NativeControlHost）
/// 2. ViewportHostControl 重写 CreateNativeControlCore，直接使用 Avalonia 的 DumbWindow
/// 3. 通过 HandleCreated 事件获取原生句柄
/// 4. 将句柄传递给 Diligent 渲染引擎
///
/// 跨平台：
/// - Windows：HWND（DumbWindow，HandleDescriptor="HWND"）
/// - Linux X11：X11 Window（HandleDescriptor="X11"）
/// - macOS：NSView（HandleDescriptor="NSView"）
/// </summary>
public class NativeControlHostSurface : IViewportSurface
{
    private ViewportHostControl? _host;
    private IntPtr _nativeHandle;
    private string _handleDescriptor = "";
    private int _width;
    private int _height;
    private bool _isValid;
    private bool _disposed;

    public int Width => _width;
    public int Height => _height;
    public bool IsValid => _isValid && !_disposed;

    public event Action<IntPtr>? SurfaceCreated;
    public event Action? SurfaceDestroyed;
    public event Action<int, int>? SurfaceResized;

    /// <summary>
    /// 创建 NativeControlHost 表面。
    /// </summary>
    /// <param name="width">初始宽度。</param>
    /// <param name="height">初始高度。</param>
    public NativeControlHostSurface(int width, int height)
    {
        _width = width;
        _height = height;

        try
        {
            _host = new ViewportHostControl();

            // 监听子窗口创建事件
            _host.HandleCreated += OnHandleCreated;
            _host.HandleDestroyed += OnHandleDestroyed;

            _isValid = true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[NativeControlHostSurface] 创建失败: {ex.Message}");
            _isValid = false;
        }
    }

    /// <summary>
    /// 获取 ViewportHostControl 控件（需要添加到 Avalonia 可视树）。
    /// </summary>
    public ViewportHostControl GetControl()
    {
        if (_host == null)
            throw new InvalidOperationException("Surface 已销毁");

        return _host;
    }

    /// <summary>子窗口创建完成回调。</summary>
    private void OnHandleCreated(Avalonia.Platform.IPlatformHandle handle)
    {
        _nativeHandle = handle.Handle;
        _handleDescriptor = handle.HandleDescriptor;
        _isValid = true;
        Console.WriteLine($"[NativeControlHostSurface] 原生句柄已获取: 0x{handle.Handle:X} ({handle.HandleDescriptor}), SurfaceCreated 订阅者数: {SurfaceCreated?.GetInvocationList().Length ?? 0}");
        SurfaceCreated?.Invoke(handle.Handle);
    }

    /// <summary>子窗口销毁回调。</summary>
    private void OnHandleDestroyed()
    {
        _nativeHandle = IntPtr.Zero;
        _handleDescriptor = "";
        _isValid = false;
        SurfaceDestroyed?.Invoke();
    }

    public IntPtr GetNativeHandle()
    {
        return _nativeHandle;
    }

    /// <summary>获取原生句柄描述符（"HWND"/"X11"/"NSView"）。</summary>
    public string GetHandleDescriptor()
    {
        return _handleDescriptor;
    }

    public void Resize(int width, int height)
    {
        if (_width == width && _height == height)
            return;

        _width = width;
        _height = height;

        SurfaceResized?.Invoke(width, height);
        Console.WriteLine($"[NativeControlHostSurface] 尺寸变更: {width}x{height}");
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        _isValid = false;

        if (_host != null)
        {
            _host.HandleCreated -= OnHandleCreated;
            _host.HandleDestroyed -= OnHandleDestroyed;
            _host = null;
        }

        _nativeHandle = IntPtr.Zero;
        SurfaceDestroyed?.Invoke();

        Console.WriteLine("[NativeControlHostSurface] 已销毁");
    }
}
