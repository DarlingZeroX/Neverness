using Neverness.Editor.Core.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 视口服务实现——封装 Native 渲染 API。
/// 对外暴露 IViewportService 接口，Controller 不直接访问 Native API。
/// </summary>
public sealed unsafe class ViewportServiceImpl : IViewportService
{
    private ulong _sceneHandle;

    /// <summary>当前关联的场景句柄。</summary>
    public ulong SceneHandle => _sceneHandle;

    /// <summary>是否有有效的场景。</summary>
    public bool HasScene => _sceneHandle != 0;

    /// <summary>设置关联的场景。</summary>
    public void SetScene(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
    }

    /// <summary>渲染场景到纹理并返回纹理 ID。</summary>
    public ulong RenderSceneToTexture(float width, float height)
    {
        if (_sceneHandle == 0 || width < 1 || height < 1) return 0;

        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.RenderSceneToTexture == null) return 0;

        var result = api.ViewportRender.RenderSceneToTexture(
            _sceneHandle,
            (uint)width,
            (uint)height);

        if (result == 0)
        {
            Console.WriteLine($"[ViewportService] RenderSceneToTexture: returned 0 (sceneHandle={_sceneHandle}, w={width}, h={height})");
        }

        return result;
    }

    /// <summary>获取最后渲染的场景纹理 ID。</summary>
    public ulong GetLastSceneTextureId()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.GetLastRenderedTexture == null) return 0;

        return api.ViewportRender.GetLastRenderedTexture();
    }

    /// <summary>获取最后渲染的 RmlUI 纹理 ID。</summary>
    public ulong GetLastRmluiTextureId()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.GetLastRmluiTexture == null) return 0;

        return api.ViewportRender.GetLastRmluiTexture();
    }

    /// <summary>聚焦到指定实体（摄像机移动）。</summary>
    /// <remarks>
    /// Native API 没有 FocusEntity 函数指针。
    /// 需要 Native 端在 NNViewportRenderApi 中添加 FocusEntity 接口。
    /// </remarks>
    public void FocusEntity(ulong entityHandle)
    {
        // Native API 不支持摄像机聚焦
        // NNViewportRenderApi 没有 FocusEntity 函数指针
        Console.WriteLine($"[ViewportService] FocusEntity 不支持: {entityHandle} (Native API 无接口)");
    }

    /// <summary>设置摄像机位置。</summary>
    /// <remarks>
    /// Native API 没有 SetCameraPosition 函数指针。
    /// 需要 Native 端在 NNViewportRenderApi 中添加 SetCameraPosition 接口。
    /// </remarks>
    public void SetCameraPosition(float x, float y, float z)
    {
        // Native API 不支持设置摄像机位置
        // NNViewportRenderApi 没有 SetCameraPosition 函数指针
        Console.WriteLine($"[ViewportService] SetCameraPosition 不支持: ({x}, {y}, {z}) (Native API 无接口)");
    }

    /// <summary>获取渲染统计信息。</summary>
    public RenderStats GetRenderStats()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;

        uint drawCalls = 0;
        uint vertices = 0;

        if (api.ViewportRender.GetRenderStats != null)
        {
            api.ViewportRender.GetRenderStats(&drawCalls, &vertices);
        }

        return new RenderStats
        {
            DrawCalls = drawCalls,
            Vertices = vertices,
            Triangles = vertices / 3 // 估算
        };
    }

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    public void SetRmlUIViewportSize(uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.SetRmlUIViewportSize != null)
        {
            api.ViewportRender.SetRmlUIViewportSize(width, height);
        }
    }
}
