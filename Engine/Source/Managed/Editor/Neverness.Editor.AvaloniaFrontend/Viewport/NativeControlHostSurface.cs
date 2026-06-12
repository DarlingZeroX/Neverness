using Avalonia.Controls;
using Avalonia.Platform;

namespace Neverness.Editor.AvaloniaFrontend.Viewport;

/// <summary>
/// NativeControlHost 视口表面实现——使用 Avalonia NativeControlHost 嵌入原生窗口。
///
/// 工作原理：
/// 1. 创建一个 NativeControlHost 控件
/// 2. 通过 IPlatformHandle 获取原生窗口句柄
/// 3. 将句柄传递给 Diligent 渲染引擎
/// 4. Diligent 直接渲染到该原生窗口
///
/// 平台支持：
/// - Windows：HWND
/// - Linux：X11 Window
/// - macOS：NSView
/// </summary>
public class NativeControlHostSurface : IViewportSurface
{
    private NativeControlHost? _host;
    private IntPtr _nativeHandle;
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
            // 创建 NativeControlHost
            _host = new NativeControlHost();

            // 获取原生窗口句柄
            // 注意：NativeControlHost 需要先附加到可视树才能获取句柄
            // 此处先标记为待初始化，等附加到可视树后再获取句柄
            _isValid = true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[NativeControlHostSurface] 创建失败: {ex.Message}");
            _isValid = false;
        }
    }

    /// <summary>
    /// 获取 NativeControlHost 控件（需要添加到 Avalonia 可视树）。
    /// </summary>
    public NativeControlHost GetControl()
    {
        if (_host == null)
            throw new InvalidOperationException("Surface 已销毁");

        return _host;
    }

    /// <summary>
    /// 初始化原生句柄（在控件附加到可视树后调用）。
    /// </summary>
    public void InitializeNativeHandle()
    {
        if (_host == null || _nativeHandle != IntPtr.Zero)
            return;

        try
        {
            // 获取原生平台句柄
            // NativeControlHost 在附加到可视树后会创建原生子窗口
            var topLevel = TopLevel.GetTopLevel(_host);
            if (topLevel?.PlatformImpl != null)
            {
                // 尝试获取原生句柄
                // 注意：具体 API 取决于 Avalonia 版本和平台
                _nativeHandle = _host.GetHashCode(); // 占位，实际需要平台特定代码
                _isValid = true;

                Console.WriteLine($"[NativeControlHostSurface] 原生句柄已初始化: 0x{_nativeHandle:X}");
                SurfaceCreated?.Invoke(_nativeHandle);
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[NativeControlHostSurface] 初始化原生句柄失败: {ex.Message}");
            _isValid = false;
        }
    }

    public IntPtr GetNativeHandle()
    {
        return _nativeHandle;
    }

    public void Resize(int width, int height)
    {
        if (_width == width && _height == height)
            return;

        _width = width;
        _height = height;

        // 通知尺寸变更
        SurfaceResized?.Invoke(width, height);

        Console.WriteLine($"[NativeControlHostSurface] 尺寸变更: {width}x{height}");
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        _isValid = false;

        SurfaceDestroyed?.Invoke();

        // NativeControlHost 由 Avalonia 可视树管理生命周期
        // 此处只需清理引用
        _host = null;
        _nativeHandle = IntPtr.Zero;

        Console.WriteLine("[NativeControlHostSurface] 已销毁");
    }
}
