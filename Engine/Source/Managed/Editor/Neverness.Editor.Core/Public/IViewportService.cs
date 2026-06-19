using Neverness.Runtime.Engine;
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

    /// <summary>
    /// 从 ECS 收集渲染命令并序列化为 Flat Buffer。
    ///
    /// 数据流：
    /// Friflo ECS (TransformComponent + SpriteRendererComponent + CameraComponent)
    ///   → SetCamera + DrawSpriteBatch 命令
    ///   → RenderCommandBuffer.Build()
    ///   → byte[]
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    /// <returns>序列化的命令缓冲区，可直接传给 RenderViewportCommands。无场景时返回 null。</returns>
    byte[]? CollectRenderCommands(float width, float height);

    /// <summary>
    /// 资产 GUID → VFS 路径解析器（由上层注入，避免 Scene 模块直接依赖 Assets 模块）。
    /// 用于 RmlUIDocument 组件的 GUID → 路径解析。返回 null 表示解析失败。
    /// </summary>
    Func<NNGuid, string?>? AssetPathResolver { get; set; }
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
