// Neverness.Runtime.Application — SDL3 窗口对象封装。
// 替代 C++ VGWindow + Window.cs，直接操作 SDL_Window*。
// 支持普通窗口和输入窗口两种模式。

using System.Runtime.InteropServices;

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// SDL3 窗口对象封装。
/// 持有 SDL_Window* 指针，提供窗口操作和原生句柄获取。
///
/// 支持两种模式：
/// 1. 普通窗口：完整的 SDL 窗口，支持渲染、输入等
/// 2. 输入窗口：绑定到 Avalonia DumbWindow，只接管输入
/// </summary>
public sealed unsafe class SdlWindow : IDisposable
{
    private bool _disposed;
    private SDL.SDL_Window* _window;

    /// <summary>窗口事件分发器。</summary>
    public SdlWindowEvents Events { get; } = new();

    /// <summary>SDL_Window 原生指针。</summary>
    public SDL.SDL_Window* SdlWindowPtr => _window;

    /// <summary>SDL_Window 原生指针（IntPtr 形式，供非 unsafe 代码使用）。</summary>
    public IntPtr NativeWindowPtr => (IntPtr)_window;

    /// <summary>窗口句柄（SDL_WindowID）。</summary>
    public WindowHandle Handle { get; }

    /// <summary>是否为主窗口。</summary>
    public bool IsPrimary { get; internal set; }

    /// <summary>是否为输入窗口（绑定到外部 HWND，只接管输入）。</summary>
    public bool IsInputOnly { get; }

    /// <summary>父窗口 HWND（仅输入窗口有效）。</summary>
    public IntPtr ParentHwnd { get; }

    /// <summary>是否有效。</summary>
    public bool IsValid => !_disposed && _window != null;

    /// <summary>平台原生窗口句柄（HWND / NSWindow / X11）。</summary>
    public nint NativeHandle
    {
        get
        {
            ThrowIfDisposed();
            return GetPlatformHandle();
        }
    }

    /// <summary>
    /// 创建普通 SDL 窗口。
    /// </summary>
    internal SdlWindow(SDL.SDL_Window* window, WindowHandle handle, bool isPrimary = false)
    {
        _window = window;
        Handle = handle;
        IsPrimary = isPrimary;
        IsInputOnly = false;
        ParentHwnd = IntPtr.Zero;
    }

    /// <summary>
    /// 创建输入窗口（绑定到外部 HWND）。
    /// </summary>
    internal SdlWindow(SDL.SDL_Window* window, WindowHandle handle, IntPtr parentHwnd)
    {
        _window = window;
        Handle = handle;
        IsPrimary = false;
        IsInputOnly = true;
        ParentHwnd = parentHwnd;
    }

    /// <summary>设置窗口标题。</summary>
    public void SetTitle(string title)
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_SetWindowTitle(_window, title);
    }

    /// <summary>设置窗口尺寸。</summary>
    public void SetSize(int width, int height)
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_SetWindowSize(_window, width, height);
    }

    /// <summary>获取窗口尺寸。</summary>
    public (int Width, int Height) GetSize()
    {
        ThrowIfDisposed();
        int w, h;
        SDL.SDL3.SDL_GetWindowSize(_window, &w, &h);
        return (w, h);
    }

    /// <summary>窗口尺寸（tuple 属性，兼容旧 API 风格）。</summary>
    public (int Width, int Height) Size
    {
        get => GetSize();
        set => SetSize(value.Width, value.Height);
    }

    /// <summary>设置窗口位置。</summary>
    public void SetPosition(int x, int y)
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_SetWindowPosition(_window, x, y);
    }

    /// <summary>获取窗口位置。</summary>
    public (int X, int Y) GetPosition()
    {
        ThrowIfDisposed();
        int x, y;
        SDL.SDL3.SDL_GetWindowPosition(_window, &x, &y);
        return (x, y);
    }

    /// <summary>窗口位置（tuple 属性，兼容旧 API 风格）。</summary>
    public (int X, int Y) Position
    {
        get => GetPosition();
        set => SetPosition(value.X, value.Y);
    }

    /// <summary>显示窗口。</summary>
    public void Show()
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_ShowWindow(_window);
    }

    /// <summary>隐藏窗口。</summary>
    public void Hide()
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_HideWindow(_window);
    }

    /// <summary>最大化窗口。</summary>
    public void Maximize()
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_MaximizeWindow(_window);
    }

    /// <summary>最小化窗口。</summary>
    public void Minimize()
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_MinimizeWindow(_window);
    }

    /// <summary>恢复窗口。</summary>
    public void Restore()
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_RestoreWindow(_window);
    }

    /// <summary>设置窗口是否可调整大小。</summary>
    public void SetResizable(bool value)
    {
        ThrowIfDisposed();
        SDL.SDL3.SDL_SetWindowResizable(_window, value);
    }

    /// <summary>获取平台原生窗口句柄（HWND / NSWindow / X11）。</summary>
    private nint GetPlatformHandle()
    {
        var props = SDL.SDL3.SDL_GetWindowProperties(_window);
        if (props == 0)
        {
            return 0;
        }

        // Windows: SDL_PROP_WINDOW_WIN32_HWND_POINTER
        // macOS: SDL_PROP_WINDOW_COCOA_WINDOW_POINTER
        // Linux: SDL_PROP_WINDOW_X11_WINDOW_NUMBER
        nint handle = SDL.SDL3.SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", 0);
        if (handle != 0)
        {
            return handle;
        }

        handle = SDL.SDL3.SDL_GetPointerProperty(props, "SDL.window.cocoa.window", 0);
        if (handle != 0)
        {
            return handle;
        }

        handle = SDL.SDL3.SDL_GetPointerProperty(props, "SDL.window.x11.window", 0);
        return handle;
    }

    /// <summary>销毁窗口。</summary>
    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        if (_window != null)
        {
            SDL.SDL3.SDL_DestroyWindow(_window);
            _window = null;
        }

        GC.SuppressFinalize(this);
    }

    ~SdlWindow()
    {
        Dispose();
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
