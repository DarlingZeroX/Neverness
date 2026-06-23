// Neverness.Runtime.Application — ImGui Backend C#/C++ 桥接。
// 调用 C++ nn_imgui_backend_* 函数，管理 ImGui SDL3/Diligent 后端。
// ImGui UI 通过 Hexa.NET.ImGui (cimgui P/Invoke) 完全托管。

using System.Runtime.InteropServices;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// ImGui Backend 桥接层。
/// C++ 端封装 ImGuiImplSDL3 + ImGuiImplDiligent，C# 端通过 P/Invoke 调用。
/// </summary>
public static unsafe class ImGuiBackendBridge
{
    // C++ ImGui Backend 函数导入
    [DllImport("NevernessRuntime-Application", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool nn_imgui_backend_initialize(
        SDL.SDL_Window* sdlWindow,
        IntPtr device,
        IntPtr context,
        IntPtr swapChain);

    [DllImport("NevernessRuntime-Application", CallingConvention = CallingConvention.Cdecl)]
    private static extern void nn_imgui_backend_shutdown();

    [DllImport("NevernessRuntime-Application", CallingConvention = CallingConvention.Cdecl)]
    private static extern void nn_imgui_backend_new_frame(
        int width,
        int height,
        int preTransform);

    [DllImport("NevernessRuntime-Application", CallingConvention = CallingConvention.Cdecl)]
    private static extern void nn_imgui_backend_render(
        IntPtr context,
        IntPtr swapChain);

    [DllImport("NevernessRuntime-Application", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool nn_imgui_backend_process_event(SDL.SDL_Event* e);

    private static bool s_initialized;

    /// <summary>是否已初始化。</summary>
    public static bool IsInitialized => s_initialized;

    /// <summary>
    /// 初始化 ImGui 后端。
    /// </summary>
    /// <param name="sdlWindow">SDL_Window 指针</param>
    /// <param name="device">Diligent IDevice* 原生指针</param>
    /// <param name="context">Diligent IDeviceContext* 原生指针</param>
    /// <param name="swapChain">Diligent ISwapChain* 原生指针</param>
    public static bool Initialize(SDL.SDL_Window* sdlWindow, IntPtr device, IntPtr context, IntPtr swapChain)
    {
        return Initialize((IntPtr)sdlWindow, device, context, swapChain);
    }

    /// <summary>
    /// 初始化 ImGui 后端（IntPtr 重载，供非 unsafe 代码使用）。
    /// </summary>
    public static bool Initialize(IntPtr sdlWindowPtr, IntPtr device, IntPtr context, IntPtr swapChain)
    {
        return false;

        if (s_initialized)
        {
            return true;
        }

        if (sdlWindowPtr == IntPtr.Zero || device == IntPtr.Zero || context == IntPtr.Zero || swapChain == IntPtr.Zero)
        {
            Console.Error.WriteLine("[ImGuiBackendBridge] 参数无效");
            return false;
        }

        if (!nn_imgui_backend_initialize((SDL.SDL_Window*)sdlWindowPtr, device, context, swapChain))
        {
            Console.Error.WriteLine("[ImGuiBackendBridge] nn_imgui_backend_initialize 失败");
            return false;
        }

        s_initialized = true;
        Console.WriteLine("[ImGuiBackendBridge] ImGui Backend 初始化成功");
        return true;
    }

    /// <summary>
    /// 关闭 ImGui 后端。
    /// </summary>
    public static void Shutdown()
    {
        if (!s_initialized)
        {
            return;
        }

        nn_imgui_backend_shutdown();
        s_initialized = false;
        Console.WriteLine("[ImGuiBackendBridge] ImGui Backend 已关闭");
    }

    /// <summary>
    /// ImGui NewFrame（SDL3 + Diligent 后端）。
    /// 在 ImGui.NewFrame() 之前调用。
    /// </summary>
    public static void NewFrame(int width, int height, int preTransform = 0)
    {
        if (!s_initialized)
        {
            return;
        }

        nn_imgui_backend_new_frame(width, height, preTransform);
    }

    /// <summary>
    /// ImGui Render（设置渲染目标 + 提交绘制数据）。
    /// 在 ImGui.Render() 之后调用。
    /// </summary>
    public static void Render(IntPtr context, IntPtr swapChain)
    {
        if (!s_initialized)
        {
            return;
        }

        nn_imgui_backend_render(context, swapChain);
    }

    /// <summary>
    /// 将 SDL_Event 传递给 ImGui 后端处理输入。
    /// </summary>
    public static bool ProcessEvent(SDL.SDL_Event e)
    {
        if (!s_initialized)
        {
            return false;
        }

        return nn_imgui_backend_process_event(&e);
    }
}
