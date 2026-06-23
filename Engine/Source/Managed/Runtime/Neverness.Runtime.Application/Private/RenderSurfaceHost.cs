// Neverness.Runtime.Application — 渲染表面管理。
// 分离 Window 和 SwapChain，支持多窗口编辑器。
// 当前阶段通过 NNDiligentApi 创建，未来迁移到 DiligentEngine.NET 直接创建。

using Neverness.Runtime.Application.Public;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// 渲染表面管理器。
/// 负责从 SDL 窗口创建 Diligent Surface/SwapChain。
/// </summary>
public static class RenderSurfaceHost
{
    private static readonly Dictionary<WindowHandle, RenderSurface> s_surfaces = new();

    /// <summary>为窗口创建渲染表面。</summary>
    public static unsafe RenderSurface? CreateSurface(WindowHandle windowHandle)
    {
        var window = SdlWindowManager.Resolve(windowHandle);
        if (window == null)
        {
            Console.Error.WriteLine($"[RenderSurfaceHost] 窗口 {windowHandle} 不存在");
            return null;
        }

        // 当前阶段：通过 NNDiligentApi 创建 Surface/SwapChain
        // 未来阶段：直接使用 DiligentEngine.NET
        if (!EngineNativeApiBootstrap.IsInstalled)
        {
            Console.Error.WriteLine("[RenderSurfaceHost] EngineApi 未安装");
            return null;
        }

        ref readonly var api = ref EngineNativeApiCache.EngineApi;

        // 步骤 1：为窗口创建 Diligent 设备（如果尚未创建）
        // 使用 CreateDeviceForNativeHandle 直接传平台原生句柄（HWND），绕过 SDL3 实例不共享问题
        if (api.Diligent.CreateDeviceForNativeHandle != null)
        {
            var (w, h) = window.GetSize();
            if (w < 1) w = 1280;
            if (h < 1) h = 720;

            // 获取平台原生窗口句柄（HWND on Windows, X11 Window on Linux, NSView on macOS）
            nint nativeHandle = window.NativeHandle;
            if (nativeHandle == 0)
            {
                Console.Error.WriteLine("[RenderSurfaceHost] 无法获取平台原生窗口句柄");
                return null;
            }

            // 根据平台确定句柄类型（与 NNNativeHandleType 枚举一致）
            uint handleType = GetNativeHandleType();

            uint result = api.Diligent.CreateDeviceForNativeHandle(
                (void*)nativeHandle,
                handleType,
                (uint)w,
                (uint)h);

            if (result == 0)
            {
                Console.Error.WriteLine("[RenderSurfaceHost] CreateDeviceForNativeHandle 失败");
                return null;
            }

            Console.WriteLine($"[RenderSurfaceHost] Diligent 设备已创建: {w}x{h}, HandleType={handleType}");
        }

        // 步骤 2：获取 Diligent 设备指针
        IntPtr device = IntPtr.Zero;
        IntPtr context = IntPtr.Zero;
        IntPtr swapChain = IntPtr.Zero;

        if (api.Diligent.GetPrimaryDevice != null)
            device = (IntPtr)api.Diligent.GetPrimaryDevice();
        if (api.Diligent.GetPrimaryContext != null)
            context = (IntPtr)api.Diligent.GetPrimaryContext();
        if (api.Diligent.GetPrimarySwapChain != null)
            swapChain = (IntPtr)api.Diligent.GetPrimarySwapChain();

        if (device == IntPtr.Zero || context == IntPtr.Zero || swapChain == IntPtr.Zero)
        {
            Console.Error.WriteLine("[RenderSurfaceHost] Diligent 设备指针无效");
            return null;
        }

        var renderSurface = new RenderSurface(device, swapChain, windowHandle)
        {
            Context = context
        };
        s_surfaces[windowHandle] = renderSurface;

        Console.WriteLine($"[RenderSurfaceHost] 渲染表面已创建: Window={windowHandle}");
        return renderSurface;
    }

    /// <summary>调整渲染表面大小。</summary>
    public static void Resize(WindowHandle handle, int width, int height)
    {
        if (s_surfaces.TryGetValue(handle, out var surface))
        {
            surface.Resize(width, height);
        }
    }

    /// <summary>销毁渲染表面。</summary>
    public static void Destroy(WindowHandle handle)
    {
        if (s_surfaces.Remove(handle, out var surface))
        {
            surface.Dispose();
        }
    }

    /// <summary>销毁所有渲染表面。</summary>
    public static void DestroyAll()
    {
        foreach (var surface in s_surfaces.Values)
        {
            surface.Dispose();
        }

        s_surfaces.Clear();
    }

    /// <summary>获取窗口的渲染表面。</summary>
    public static RenderSurface? GetSurface(WindowHandle handle)
    {
        return s_surfaces.TryGetValue(handle, out var surface) ? surface : null;
    }

    /// <summary>根据当前平台返回 NNNativeHandleType 枚举值。</summary>
    private static uint GetNativeHandleType()
    {
        if (OperatingSystem.IsWindows()) return 0; // Win32HWND
        if (OperatingSystem.IsLinux())   return 1; // X11Window
        if (OperatingSystem.IsMacOS())   return 3; // NSView
        return 0;
    }
}
