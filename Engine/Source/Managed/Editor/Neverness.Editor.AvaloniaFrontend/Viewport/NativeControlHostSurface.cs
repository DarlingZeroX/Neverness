using Avalonia.Controls;
using Neverness.Rendering.Core;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.AvaloniaFrontend.Viewport;

/// <summary>
/// NativeControlHost 视口表面实现——使用 ViewportHostControl 嵌入原生窗口。
///
/// 工作原理：
/// 1. 创建 ViewportHostControl（子类化 NativeControlHost）
/// 2. ViewportHostControl 重写 CreateNativeControlCore，直接使用 Avalonia 的 DumbWindow
/// 3. 通过 HandleCreated 事件获取原生句柄
/// 4. 创建 SDL 输入窗口（绑定到 HWND，只接管输入）
/// 5. 创建 ViewportId 统一管理 WindowHandle + SurfaceId + IViewportService
/// 6. 将句柄传递给 Diligent 渲染引擎
///
/// 跨平台：
/// - Windows：HWND（DumbWindow，HandleDescriptor="HWND"）
/// - Linux X11：X11 Window（HandleDescriptor="X11"）
/// - macOS：NSView（HandleDescriptor="NSView"）
///
/// SDL 输入窗口：
/// - 使用 SDL_CreateWindowWithProperties 创建
/// - 设置 SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER 绑定 HWND
/// - 只接管输入（键盘、鼠标、游戏手柄），不管理窗口生命周期
/// - 由 SdlWindowManager 统一管理
///
/// ViewportId：
/// - 统一管理 WindowHandle、SurfaceId、IViewportService
/// - 由 ViewportIdManager 管理
/// - 用于 SDL 事件路由和渲染调用
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

    // ── ViewportId（统一管理 WindowHandle + SurfaceId + IViewportService） ──
    private ViewportId _viewportId;

    public int Width => _width;
    public int Height => _height;
    public bool IsValid => _isValid && !_disposed;

    /// <summary>ViewportId（统一管理 WindowHandle、SurfaceId、IViewportService）。</summary>
    public ViewportId ViewportId => _viewportId;

    /// <summary>SDL 窗口句柄（SDL_WindowID）。</summary>
    public WindowHandle WindowHandle => _viewportId.WindowHandle;

    /// <summary>渲染表面 ID（IViewportSurfaceRegistry.Register() 返回）。</summary>
    public ulong SurfaceId => _viewportId.SurfaceId;

    public event Action<ViewportId>? SurfaceCreated;
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
        Console.WriteLine($"[NativeControlHostSurface] 原生句柄已获取: 0x{handle.Handle:X} ({handle.HandleDescriptor})");

        // 初始化 SDL 输入窗口（跨平台支持）
        if (handle.HandleDescriptor is "HWND" or "NSView" or "X11")
        {
            var sdlInputHandle = SdlWindowManager.CreateInputWindow(handle.Handle, handle.HandleDescriptor, _width, _height);
            if (sdlInputHandle.IsValid)
            {
                // 创建 ViewportId（SurfaceId 暂时为 0，后续由 ViewportAvaloniaView 设置）
                _viewportId = new ViewportId(sdlInputHandle, 0, null);

                // 注册到 ViewportIdManager
                ViewportIdManager.Register(_viewportId);

                Console.WriteLine($"[NativeControlHostSurface] SDL 输入窗口初始化成功: {sdlInputHandle}");
                Console.WriteLine($"[NativeControlHostSurface] ViewportId 已创建: {_viewportId}");
            }
            else
            {
                Console.Error.WriteLine("[NativeControlHostSurface] SDL 输入窗口初始化失败");
            }
        }
        else
        {
            Console.WriteLine($"[NativeControlHostSurface] 不支持的句柄类型: {handle.HandleDescriptor}，跳过 SDL 输入窗口创建");
        }

        SurfaceCreated?.Invoke(_viewportId);
    }

    /// <summary>子窗口销毁回调。</summary>
    private void OnHandleDestroyed()
    {
        // 注销 ViewportId
        if (_viewportId.IsValid)
        {
            ViewportIdManager.Unregister(_viewportId);

            // 销毁 SDL 输入窗口
            SdlWindowManager.Destroy(_viewportId.WindowHandle);

            Console.WriteLine($"[NativeControlHostSurface] ViewportId 已注销: {_viewportId}");
            _viewportId = ViewportId.Invalid;
        }

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

        // 更新 SDL 输入窗口尺寸
        if (_viewportId.IsValid)
        {
            SdlWindowManager.UpdateInputWindowSize(_viewportId.WindowHandle, width, height);
        }

        SurfaceResized?.Invoke(width, height);
        Console.WriteLine($"[NativeControlHostSurface] 尺寸变更: {width}x{height}");
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        _isValid = false;

        // 注销 ViewportId 并销毁 SDL 输入窗口
        if (_viewportId.IsValid)
        {
            ViewportIdManager.Unregister(_viewportId);
            SdlWindowManager.Destroy(_viewportId.WindowHandle);
            Console.WriteLine($"[NativeControlHostSurface] ViewportId 已注销: {_viewportId}");
            _viewportId = ViewportId.Invalid;
        }

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
