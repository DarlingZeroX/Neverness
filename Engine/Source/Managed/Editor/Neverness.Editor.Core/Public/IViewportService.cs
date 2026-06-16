using Neverness.Runtime.Scene;

namespace Neverness.Editor.Core.Public;

/// <summary>
/// 视口服务接口——提供场景渲染和视口操作。
/// Viewport 模块实现，Controller 通过服务定位器消费。
/// </summary>
public interface IViewportService
{
    /// <summary>是否有有效的场景。</summary>
    bool HasScene { get; }

    /// <summary>渲染场景到纹理并返回纹理 ID。</summary>
    ulong RenderSceneToTexture(float width, float height);

    /// <summary>获取最后渲染的场景纹理 ID。</summary>
    ulong GetLastSceneTextureId();

    /// <summary>获取最后渲染的 RmlUI 纹理 ID。</summary>
    ulong GetLastRmluiTextureId();

    /// <summary>设置关联的场景。</summary>
    void SetScene(SceneWorld? scene);

    /// <summary>聚焦到指定实体（摄像机移动）。</summary>
    void FocusEntity(IEntity entity);

    /// <summary>设置摄像机位置。</summary>
    void SetCameraPosition(float x, float y, float z);

    /// <summary>获取渲染统计信息。</summary>
    RenderStats GetRenderStats();

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    void SetRmlUIViewportSize(uint width, uint height);
}

/// <summary>
/// 渲染统计信息。
/// </summary>
public struct RenderStats
{
    public uint DrawCalls;
    public uint Vertices;
    public uint Triangles;
}
