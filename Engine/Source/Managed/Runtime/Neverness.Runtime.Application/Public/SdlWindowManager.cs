// Neverness.Runtime.Application — SDL3 窗口注册表。
// 替代 C++ WindowRegistry，直接管理 SDL_Window*。
// 统一管理普通窗口和输入窗口。

using System.Diagnostics;
using Neverness.Runtime.Application.Public;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// SDL3 窗口注册表。
/// 使用 SDL_WindowID (uint32_t) 作为唯一标识，无需自增 ulong。
///
/// 统一管理：
/// - 普通窗口：完整的 SDL 窗口，支持渲染、输入等
/// - 输入窗口：绑定到 Avalonia DumbWindow，只接管输入
/// </summary>
public static unsafe class SdlWindowManager
{
    private static readonly Dictionary<WindowHandle, SdlWindow> s_windows = new();
    private static WindowHandle s_primaryHandle;

    /// <summary>主窗口句柄。</summary>
    public static WindowHandle PrimaryHandle => s_primaryHandle;

    /// <summary>当前已创建的窗口数量。</summary>
    public static int Count => s_windows.Count;

    /// <summary>
    /// 创建普通 SDL 窗口。
    /// 第一个创建的窗口自动成为主窗口。
    /// </summary>
    public static WindowHandle Create(string title, int width, int height,
        bool resizable = true, bool maximized = false, bool hidden = false)
    {
        if (!SdlApplicationHost.IsInitialized)
        {
            Console.Error.WriteLine("[SdlWindowManager] SDL 未初始化，无法创建窗口");
            return WindowHandle.Invalid;
        }

        // 构建窗口标志
        SDL.SDL_WindowFlags flags = SDL.SDL_WindowFlags.SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (resizable) flags |= SDL.SDL_WindowFlags.SDL_WINDOW_RESIZABLE;
        if (maximized) flags |= SDL.SDL_WindowFlags.SDL_WINDOW_MAXIMIZED;
        if (hidden) flags |= SDL.SDL_WindowFlags.SDL_WINDOW_HIDDEN;

        var sdlWindow = SDL.SDL3.SDL_CreateWindow(title, width, height, flags);
        if (sdlWindow == null)
        {
            Console.Error.WriteLine($"[SdlWindowManager] SDL_CreateWindow 失败: {SDL.SDL3.SDL_GetError()}");
            return WindowHandle.Invalid;
        }

        // 使用 SDL_WindowID 作为句柄
        uint sdlId = (uint)SDL.SDL3.SDL_GetWindowID(sdlWindow);
        var handle = new WindowHandle(sdlId);

        bool isPrimary = s_windows.Count == 0;
        var window = new SdlWindow(sdlWindow, handle, isPrimary);
        s_windows[handle] = window;

        if (isPrimary)
        {
            s_primaryHandle = handle;
        }

        Console.WriteLine(SDL.SDL3.SDL_VERSION);
        Console.WriteLine($"[SdlWindowManager] 普通窗口已创建: ID={sdlId}, Primary={isPrimary}, Title=\"{title}\"");
        return handle;
    }

    /// <summary>
    /// 创建 SDL 输入窗口（绑定到外部窗口，只接管输入）。
    ///
    /// 工作原理：
    /// 1. 接收 Avalonia 的原生窗口句柄（HWND/NSView/X11 Window）
    /// 2. 使用 SDL_CreateWindowWithProperties 创建 SDL 窗口
    /// 3. 根据平台设置对应的属性
    /// 4. 设置 hidden=true 和 focusable=false
    /// 5. SDL 事件泵接收输入事件
    ///
    /// 跨平台支持：
    /// - Windows：使用 SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER
    /// - macOS：使用 SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER
    /// - Linux：使用 SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER
    ///
    /// 前置条件：
    /// - SdlApplicationHost 已初始化
    /// - nativeHandle 是有效的原生窗口句柄
    /// </summary>
    /// <param name="nativeHandle">Avalonia 原生窗口句柄（HWND/NSView/X11 Window）</param>
    /// <param name="handleDescriptor">句柄类型描述符（"HWND"/"NSView"/"X11"）</param>
    /// <param name="width">初始宽度（像素）</param>
    /// <param name="height">初始高度（像素）</param>
    /// <returns>窗口句柄，失败返回 WindowHandle.Invalid</returns>
    public static WindowHandle CreateInputWindow(IntPtr nativeHandle, string handleDescriptor, int width, int height)
    {
        if (!SdlApplicationHost.IsInitialized)
        {
            Console.Error.WriteLine("[SdlWindowManager] SDL 未初始化，无法创建输入窗口");
            return WindowHandle.Invalid;
        }

        if (nativeHandle == IntPtr.Zero)
        {
            Console.Error.WriteLine("[SdlWindowManager] nativeHandle 无效");
            return WindowHandle.Invalid;
        }

        // 创建属性列表
        var props = SDL.SDL3.SDL_CreateProperties();
        if (props == 0)
        {
            Console.Error.WriteLine("[SdlWindowManager] SDL_CreateProperties 失败");
            return WindowHandle.Invalid;
        }

        try
        {
            // 根据平台设置对应的属性
            switch (handleDescriptor)
            {
                case "HWND":
                    // Windows：使用 SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER
                    SDL.SDL3.SDL_SetPointerProperty(props, SDL.SDL3.SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, nativeHandle);
                    Console.WriteLine($"[SdlWindowManager] 设置 Win32 HWND: 0x{nativeHandle:X}");
                    break;

                case "NSView":
                    // macOS：使用 SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER
                    SDL.SDL3.SDL_SetPointerProperty(props, SDL.SDL3.SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER, nativeHandle);
                    Console.WriteLine($"[SdlWindowManager] 设置 Cocoa NSView: 0x{nativeHandle:X}");
                    break;

                case "X11":
                    // Linux：使用 SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER
                    // X11 Window ID 是 unsigned long，需要转换
                    SDL.SDL3.SDL_SetNumberProperty(props, SDL.SDL3.SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, (long)(ulong)nativeHandle);
                    Console.WriteLine($"[SdlWindowManager] 设置 X11 Window: 0x{nativeHandle:X}");
                    break;

                default:
                    Console.Error.WriteLine($"[SdlWindowManager] 不支持的句柄类型: {handleDescriptor}");
                    return WindowHandle.Invalid;
            }

            // 创建 SDL 窗口
            var sdlWindow = SDL.SDL3.SDL_CreateWindowWithProperties(props);
            if (sdlWindow == null)
            {
                Console.Error.WriteLine($"[SdlWindowManager] SDL_CreateWindowWithProperties 失败: {SDL.SDL3.SDL_GetError()}");
                return WindowHandle.Invalid;
            }

            // 获取 SDL_WindowID
            uint sdlId = (uint)SDL.SDL3.SDL_GetWindowID(sdlWindow);
            var handle = new WindowHandle(sdlId);

            // 创建输入窗口实例
            var window = new SdlWindow(sdlWindow, handle, nativeHandle);
            s_windows[handle] = window;

            Console.WriteLine($"[SdlWindowManager] 输入窗口已创建: ID={sdlId}, Platform={handleDescriptor}, Handle=0x{nativeHandle:X}, Size={width}x{height}");
            return handle;
        }
        finally
        {
            // 销毁属性列表
            SDL.SDL3.SDL_DestroyProperties(props);
        }
    }

    /// <summary>销毁窗口。</summary>
    public static void Destroy(WindowHandle handle)
    {
        if (!handle.IsValid || !s_windows.Remove(handle, out var window))
        {
            return;
        }

        window.Dispose();

        // 如果销毁的是主窗口，选择新的主窗口
        if (handle == s_primaryHandle)
        {
            s_primaryHandle = s_windows.Count > 0
                ? s_windows.Keys.First()
                : WindowHandle.Invalid;
        }
    }

    /// <summary>销毁所有窗口。</summary>
    public static void DestroyAll()
    {
        foreach (var window in s_windows.Values)
        {
            window.Dispose();
        }

        s_windows.Clear();
        s_primaryHandle = WindowHandle.Invalid;
    }

    /// <summary>解析窗口句柄，返回 SdlWindow 对象。</summary>
    public static SdlWindow? Resolve(WindowHandle handle)
    {
        return s_windows.TryGetValue(handle, out var window) ? window : null;
    }

    /// <summary>获取主窗口。</summary>
    public static SdlWindow? GetPrimaryWindow()
    {
        return s_primaryHandle.IsValid ? Resolve(s_primaryHandle) : null;
    }

    /// <summary>通过 SDL_WindowID 查找句柄（O(1) 字典查找）。</summary>
    public static WindowHandle FindBySdlId(uint sdlWindowId)
    {
        var handle = new WindowHandle(sdlWindowId);
        return s_windows.ContainsKey(handle) ? handle : WindowHandle.Invalid;
    }

    /// <summary>
    /// 更新输入窗口尺寸。
    ///
    /// 调用时机：
    /// - Avalonia 布局变化导致 DumbWindow 尺寸变化时
    /// - 由 NativeControlHostSurface.Resize() 调用
    /// </summary>
    /// <param name="handle">输入窗口句柄</param>
    /// <param name="width">新宽度（像素）</param>
    /// <param name="height">新高度（像素）</param>
    public static void UpdateInputWindowSize(WindowHandle handle, int width, int height)
    {
        var window = Resolve(handle);
        if (window == null || !window.IsInputOnly)
        {
            return;
        }

        if (width <= 0 || height <= 0)
        {
            Console.WriteLine($"[SdlWindowManager] UpdateInputWindowSize 忽略无效尺寸: {width}x{height}");
            return;
        }

        window.SetSize(width, height);
        Console.WriteLine($"[SdlWindowManager] 输入窗口尺寸已更新: ID={handle.Value}, Size={width}x{height}");
    }

    /// <summary>
    /// 获取所有输入窗口。
    /// </summary>
    /// <returns>输入窗口列表</returns>
    public static IEnumerable<SdlWindow> GetInputWindows()
    {
        return s_windows.Values.Where(w => w.IsInputOnly);
    }

    /// <summary>
    /// 获取所有普通窗口。
    /// </summary>
    /// <returns>普通窗口列表</returns>
    public static IEnumerable<SdlWindow> GetNormalWindows()
    {
        return s_windows.Values.Where(w => !w.IsInputOnly);
    }
}
