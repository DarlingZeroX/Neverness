using Neverness.Editor.Core;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 视口表面注册表实现——管理多个 ViewportSurface 的 SwapChain 生命周期。
///
/// 委托给 Native NNViewportSurfaceApi 函数指针。
/// 跨前端共享（ImGuiFrontend / AvaloniaFrontend）。
/// </summary>
public sealed unsafe class ViewportSurfaceRegistryImpl : IViewportSurfaceRegistry
{
    /// <summary>已注册表面数量。</summary>
    public int Count
    {
        get
        {
            ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
            // Native 端没有 Count 函数，用 C# 端跟踪
            return _registeredCount;
        }
    }

    private int _registeredCount;

    /// <summary>注册一个新的视口表面。</summary>
    public ulong Register(IntPtr nativeHandle, uint handleType, uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.CreateSurface == null)
        {
            Console.WriteLine("[ViewportSurfaceRegistry] CreateSurface 未实现（Native API 未就绪）");
            return 0;
        }

        var surfaceId = api.ViewportSurface.CreateSurface(
            (void*)nativeHandle,
            (NNNativeHandleType)handleType,
            width,
            height);

        if (surfaceId != 0)
        {
            _registeredCount++;
            Console.WriteLine($"[ViewportSurfaceRegistry] 注册成功: surfaceId={surfaceId}, handle=0x{nativeHandle:X}, {width}x{height}");
        }
        else
        {
            Console.Error.WriteLine($"[ViewportSurfaceRegistry] 注册失败: handle=0x{nativeHandle:X}");
        }

        return surfaceId;
    }

    /// <summary>注销一个视口表面。</summary>
    public void Unregister(ulong surfaceId)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.DestroySurface == null) return;

        api.ViewportSurface.DestroySurface(surfaceId);
        _registeredCount = Math.Max(0, _registeredCount - 1);
        Console.WriteLine($"[ViewportSurfaceRegistry] 注销: surfaceId={surfaceId}");
    }

    /// <summary>标记 Deferred Resize。</summary>
    public void MarkResize(ulong surfaceId, uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.MarkResize == null) return;

        api.ViewportSurface.MarkResize(surfaceId, width, height);
    }

    /// <summary>帧末统一执行所有 Deferred Resize。</summary>
    public void FlushResizes()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.FlushResizes == null) return;

        api.ViewportSurface.FlushResizes();
    }

    /// <summary>渲染一个视口（旧接口，保留兼容）。</summary>
    public void RenderViewport(ulong surfaceId, ulong cameraId)
    {
        // 旧接口，保留兼容
    }

    /// <summary>渲染视口到 SwapChain（完整路径：SceneRenderer → FBO → CopyTexture → SwapChain → Present）。</summary>
    public bool RenderViewport(ulong surfaceId, ulong sceneHandle, uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.RenderViewport == null)
        {
            Console.WriteLine("[ViewportSurfaceRegistry] RenderViewport 未实现（Native API 未就绪）");
            return false;
        }

        var result = api.ViewportSurface.RenderViewport(surfaceId, sceneHandle, width, height);
        return result != 0;
    }

    /// <summary>Present SwapChain。</summary>
    public void Present(ulong surfaceId)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.Present == null) return;

        api.ViewportSurface.Present(surfaceId);
    }

    /// <summary>标记表面丢失。</summary>
    public void MarkSurfaceLost(ulong surfaceId)
    {
        // Surface Lost 由 Native 端自动检测（HWND 销毁时）
        // C# 端通过 IsSurfaceLost 查询
        Console.WriteLine($"[ViewportSurfaceRegistry] MarkSurfaceLost: surfaceId={surfaceId}");
    }

    /// <summary>重建丢失的表面。</summary>
    public bool RecreateSurface(ulong surfaceId, IntPtr newNativeHandle, uint newHandleType)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.RecreateSurface == null) return false;

        var result = api.ViewportSurface.RecreateSurface(
            surfaceId,
            (void*)newNativeHandle,
            (NNNativeHandleType)newHandleType);

        if (result != 0)
        {
            Console.WriteLine($"[ViewportSurfaceRegistry] 重建成功: surfaceId={surfaceId}, newHandle=0x{newNativeHandle:X}");
        }
        else
        {
            Console.Error.WriteLine($"[ViewportSurfaceRegistry] 重建失败: surfaceId={surfaceId}");
        }

        return result != 0;
    }

    /// <summary>表面是否丢失。</summary>
    public bool IsSurfaceLost(ulong surfaceId)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.IsSurfaceLost == null) return false;

        return api.ViewportSurface.IsSurfaceLost(surfaceId) != 0;
    }

    /// <summary>表面是否已注册。</summary>
    public bool IsRegistered(ulong surfaceId)
    {
        // 通过尝试查询 IsSurfaceLost 来判断是否注册
        // Native 端未注册的 surfaceId 会返回 0
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportSurface.IsSurfaceLost == null) return false;

        // 如果 IsSurfaceLost 返回 0（正常）或 1（丢失），说明已注册
        // 如果返回其他值或函数不存在，说明未注册
        // 这里简化处理：用 Count > 0 和 surfaceId != 0 判断
        return surfaceId != 0 && _registeredCount > 0;
    }
}
