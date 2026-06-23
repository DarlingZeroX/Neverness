// Neverness.Runtime.Application — SDL3 窗口注册表。
// 替代 C++ WindowRegistry，直接管理 SDL_Window*。

using Neverness.Runtime.Application.Public;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// SDL3 窗口注册表。
/// 使用 SDL_WindowID (uint32_t) 作为唯一标识，无需自增 ulong。
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
    /// 创建 SDL 窗口。
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

        Console.WriteLine($"[SdlWindowManager] 窗口已创建: ID={sdlId}, Primary={isPrimary}, Title=\"{title}\"");
        return handle;
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

    /// <summary>通过 SDL_WindowID 查找句柄。</summary>
    public static WindowHandle FindBySdlId(uint sdlWindowId)
    {
        foreach (var (handle, window) in s_windows)
        {
            if ((uint)SDL.SDL3.SDL_GetWindowID(window.SdlWindowPtr) == sdlWindowId)
            {
                return handle;
            }
        }

        return WindowHandle.Invalid;
    }
}
