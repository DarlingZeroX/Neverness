using Neverness.Editor.Core.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 视口服务实现——封装 Native 渲染 API。
/// 对外暴露 IViewportService 接口，Controller 不直接访问 Native API。
/// </summary>
public sealed unsafe class ViewportServiceImpl : IViewportService
{
    private SceneWorld? _scene;

    /// <summary>当前关联的场景。</summary>
    public SceneWorld? Scene => _scene;

    /// <summary>是否有有效的场景。</summary>
    public bool HasScene => _scene != null;

    /// <summary>设置关联的场景。</summary>
    public void SetScene(SceneWorld? scene)
    {
        _scene = scene;
    }

    /// <summary>渲染场景到纹理并返回纹理 ID。</summary>
    public ulong RenderSceneToTexture(float width, float height)
    {
        if (_scene == null || width < 1 || height < 1) return 0;

        // TODO: 更新为使用新的 Scene API
        // 暂时返回 0
        return 0;
    }

    /// <summary>获取最后渲染的场景纹理 ID。</summary>
    public ulong GetLastSceneTextureId()
    {
        // TODO: 更新为使用新的 Scene API
        return 0;
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
    public void FocusEntity(IEntity entity)
    {
        // Native API 不支持摄像机聚焦
        // NNViewportRenderApi 没有 FocusEntity 函数指针
        Console.WriteLine($"[ViewportService] FocusEntity 不支持: {entity.Id} (Native API 无接口)");
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
