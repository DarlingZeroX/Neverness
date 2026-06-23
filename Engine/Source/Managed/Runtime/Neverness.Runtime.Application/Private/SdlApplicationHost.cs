// Neverness.Runtime.Application — SDL3 生命周期管理（C# 直接调用 SDL3-CS）。
// 替代 C++ RuntimeApplication::Initialize/Shutdown 的 SDL 部分。

using System.Runtime.InteropServices;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// SDL3 生命周期管理。
/// 通过 ppy/SDL3-CS 直接调用 SDL3 原生函数，不再经过 C++ NNApplicationApi。
/// </summary>
internal static unsafe class SdlApplicationHost
{
    private static bool s_initialized;

    /// <summary>SDL 是否已初始化。</summary>
    public static bool IsInitialized => s_initialized;

    /// <summary>
    /// 初始化 SDL3 子系统（Video + Events + Audio）。
    /// </summary>
    public static bool Initialize()
    {
        if (s_initialized)
        {
            return true;
        }

        // 设置 locale 为 UTF-8（与 C++ 端一致）
        SDL.SDL3.SDL_SetHint("SDL_HINT_WINDOWS_DPI_AWARENESS", "permonitorv2");

        if (!SDL.SDL3.SDL_Init(
                SDL.SDL_InitFlags.SDL_INIT_VIDEO |
                SDL.SDL_InitFlags.SDL_INIT_EVENTS |
                SDL.SDL_InitFlags.SDL_INIT_AUDIO))
        {
            Console.Error.WriteLine($"[SdlApplicationHost] SDL_Init 失败: {SDL.SDL3.SDL_GetError()}");
            return false;
        }

        s_initialized = true;
        Console.WriteLine("[SdlApplicationHost] SDL3 初始化成功");
        return true;
    }

    /// <summary>
    /// 关闭 SDL3 子系统。
    /// </summary>
    public static void Shutdown()
    {
        if (!s_initialized)
        {
            return;
        }

        SDL.SDL3.SDL_Quit();
        s_initialized = false;
        Console.WriteLine("[SdlApplicationHost] SDL3 已关闭");
    }
}
