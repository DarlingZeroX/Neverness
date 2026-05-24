using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景实体工厂——提供常用实体类型的快捷创建方法，类似 Unity 的 GameObject 工厂。
/// 每个方法创建实体 + 挂载预设组件 + 写入合理默认值。
/// 后续可扩展 CreateDirectionalLight、CreateSprite 等。
/// </summary>
public static class EntityFactory
{
    /// <summary>
    /// 创建 Camera 实体：自动挂载 <see cref="NNTransformData"/> + <see cref="NNCameraComponentData"/>。
    /// 类似 Unity 中 new GameObject + AddComponent&lt;Camera&gt; 的效果。
    /// </summary>
    /// <param name="world">目标场景世界。</param>
    /// <param name="displayName">显示名称（默认 "Camera"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="fovY">垂直视野角度（默认 60°）。</param>
    /// <param name="aspectRatio">宽高比（默认 16:9）。</param>
    /// <param name="nearPlane">近裁面（默认 0.3）。</param>
    /// <param name="farPlane">远裁面（默认 1000）。</param>
    /// <returns>配置好的实体；创建失败时返回 null。</returns>
    public static SceneEntity? CreateCamera(
        SceneWorld world,
        string displayName = "Camera",
        NNVec3? position = null,
        float fovY = 60.0f,
        float aspectRatio = 16.0f / 9.0f,
        float nearPlane = 0.3f,
        float farPlane = 1000.0f)
    {
        ArgumentNullException.ThrowIfNull(world);

        var entity = world.Entities.Create(displayName);
        if (entity == null)
        {
            return null;
        }

        // Transform — Identity 默认值（位置原点、无旋转、缩放 1）
        entity.AddComponent<NNTransformData>();
        entity.SetComponent(new NNTransformData
        {
            Position = position ?? default,
            Rotation = new NNQuat { W = 1.0f }, // 单位四元数（无旋转）
            Scale = new NNVec3 { X = 1, Y = 1, Z = 1 },
        });

        // Camera — 透视投影，默认参数
        entity.AddComponent<NNCameraComponentData>();
        entity.SetComponent(new NNCameraComponentData
        {
            Projection = NNProjectionType.Perspective,
            FovY = fovY,
            AspectRatio = aspectRatio,
            NearPlane = nearPlane,
            FarPlane = farPlane,
        });

        return entity;
    }
}
