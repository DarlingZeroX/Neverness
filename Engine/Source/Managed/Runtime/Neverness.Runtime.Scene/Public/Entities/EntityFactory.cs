using System.Numerics;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景实体工厂——提供常用实体类型的快捷创建方法，类似 Unity 的 GameObject 工厂。
/// 每个方法创建实体 + 挂载预设组件 + 写入合理默认值。
/// </summary>
public static class EntityFactory
{
    /// <summary>
    /// 创建 Camera 实体：自动挂载 TransformComponent + CameraComponent。
    /// 类似 Unity 中 new GameObject + AddComponent&lt;Camera&gt; 的效果。
    /// </summary>
    /// <param name="scene">目标场景。</param>
    /// <param name="displayName">显示名称（默认 "Camera"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="fovY">垂直视野角度（默认 60°）。</param>
    /// <param name="aspectRatio">宽高比（默认 16:9）。</param>
    /// <param name="nearPlane">近裁面（默认 0.1）。</param>
    /// <param name="farPlane">远裁面（默认 1000）。</param>
    /// <returns>配置好的实体。</returns>
    public static IEntity CreateCamera(
        IScene scene,
        string displayName = "Camera",
        Vector3? position = null,
        float fovY = 60.0f,
        float aspectRatio = 16.0f / 9.0f,
        float nearPlane = 0.1f,
        float farPlane = 1000.0f)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        // Transform
        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // Camera
        entity.Add(new CameraComponent
        {
            FieldOfView = fovY * MathF.PI / 180f, // 转换为弧度
            AspectRatio = aspectRatio,
            NearPlane = nearPlane,
            FarPlane = farPlane,
            IsOrthographic = false,
            OrthographicSize = 10f,
        });

        return entity;
    }

    /// <summary>
    /// 创建正交摄像机实体。
    /// </summary>
    public static IEntity CreateOrthographicCamera(
        IScene scene,
        string displayName = "Camera",
        Vector3? position = null,
        float orthographicSize = 10.0f,
        float aspectRatio = 16.0f / 9.0f,
        float nearPlane = 0.1f,
        float farPlane = 1000.0f)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        entity.Add(new CameraComponent
        {
            FieldOfView = 60f * MathF.PI / 180f,
            AspectRatio = aspectRatio,
            NearPlane = nearPlane,
            FarPlane = farPlane,
            IsOrthographic = true,
            OrthographicSize = orthographicSize,
        });

        return entity;
    }

    /// <summary>
    /// 创建 Sprite 实体：自动挂载 TransformComponent + SpriteRendererComponent。
    /// 类似 Unity 中 new GameObject + AddComponent&lt;SpriteRenderer&gt; 的效果。
    /// </summary>
    /// <param name="scene">目标场景。</param>
    /// <param name="displayName">显示名称（默认 "Sprite"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="colorR">Tint 红色分量（默认 1）。</param>
    /// <param name="colorG">Tint 绿色分量（默认 1）。</param>
    /// <param name="colorB">Tint 蓝色分量（默认 1）。</param>
    /// <param name="colorA">Tint Alpha 分量（默认 1，不透明）。</param>
    /// <param name="layer">渲染层级（默认 0）。</param>
    /// <param name="sortOrder">层级内排序（默认 0）。</param>
    /// <returns>配置好的实体。</returns>
    public static IEntity CreateSprite(
        IScene scene,
        string displayName = "Sprite",
        Vector3? position = null,
        float colorR = 1.0f,
        float colorG = 1.0f,
        float colorB = 1.0f,
        float colorA = 1.0f,
        uint layer = 0,
        uint sortOrder = 0)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        // Transform
        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // SpriteRenderer
        entity.Add(new SpriteRendererComponent
        {
            ColorR = colorR,
            ColorG = colorG,
            ColorB = colorB,
            ColorA = colorA,
            UvU0 = 0f,
            UvV0 = 0f,
            UvU1 = 1f,
            UvV1 = 1f,
            Layer = layer,
            SortOrder = sortOrder,
            Blend = BlendMode.Alpha,
            Flags = SpriteFlags.Visible,
        });

        return entity;
    }

    /// <summary>
    /// 创建 AudioSource 实体：自动挂载 TransformComponent + AudioSourceComponent。
    /// </summary>
    /// <param name="scene">目标场景。</param>
    /// <param name="displayName">显示名称（默认 "Audio Source"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="volume">音量（默认 1.0）。</param>
    /// <param name="pitch">音调（默认 1.0）。</param>
    /// <param name="minDistance">最小衰减距离（默认 1.0）。</param>
    /// <param name="maxDistance">最大衰减距离（默认 100.0）。</param>
    /// <param name="flags">标志位（默认 AutoPlay）。</param>
    /// <returns>配置好的实体。</returns>
    public static IEntity CreateAudioSource(
        IScene scene,
        string displayName = "Audio Source",
        Vector3? position = null,
        float volume = 1.0f,
        float pitch = 1.0f,
        float minDistance = 1.0f,
        float maxDistance = 100.0f,
        AudioSourceFlags flags = AudioSourceFlags.AutoPlay)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        // Transform
        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // AudioSource
        entity.Add(new AudioSourceComponent
        {
            Volume = volume,
            Pitch = pitch,
            MinDistance = minDistance,
            MaxDistance = maxDistance,
            Flags = flags,
        });

        return entity;
    }

    /// <summary>
    /// 创建 VideoPlayer 实体：自动挂载 TransformComponent + VideoPlayerComponent。
    /// </summary>
    /// <param name="scene">目标场景。</param>
    /// <param name="displayName">显示名称（默认 "Video Player"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="volume">音量（默认 1.0）。</param>
    /// <param name="flags">标志位（默认 AutoPlay）。</param>
    /// <returns>配置好的实体。</returns>
    public static IEntity CreateVideoPlayer(
        IScene scene,
        string displayName = "Video Player",
        Vector3? position = null,
        float volume = 1.0f,
        VideoPlayerFlags flags = VideoPlayerFlags.AutoPlay)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        // Transform
        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // VideoPlayer
        entity.Add(new VideoPlayerComponent
        {
            Volume = volume,
            Flags = flags,
        });

        return entity;
    }

    /// <summary>
    /// 创建 RmlUI 文档实体：自动挂载 TransformComponent + RmlUIDocumentComponent。
    /// </summary>
    /// <param name="scene">目标场景。</param>
    /// <param name="displayName">显示名称（默认 "RmlUI Document"）。</param>
    /// <param name="position">初始位置（默认原点）。</param>
    /// <param name="flags">标志位（默认 AutoLoad + ReceivesInput）。</param>
    /// <param name="sortOrder">渲染排序（默认 0）。</param>
    /// <param name="viewTarget">视图目标（默认 Both）。</param>
    /// <returns>配置好的实体。</returns>
    public static IEntity CreateRmlUIDocument(
        IScene scene,
        string displayName = "RmlUI Document",
        Vector3? position = null,
        RmlUIDocumentFlags flags = RmlUIDocumentFlags.AutoLoad | RmlUIDocumentFlags.ReceivesInput,
        int sortOrder = 0,
        RmlUIViewTarget viewTarget = RmlUIViewTarget.Both)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        // Transform
        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // RmlUIDocument
        entity.Add(new RmlUIDocumentComponent
        {
            Flags = flags,
            SortOrder = sortOrder,
            ViewTarget = viewTarget,
        });

        return entity;
    }

    /// <summary>
    /// 创建空实体（仅 TransformComponent）。
    /// </summary>
    public static IEntity CreateEmpty(
        IScene scene,
        string displayName = "Empty",
        Vector3? position = null,
        Quaternion? rotation = null,
        Vector3? scale = null)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = rotation ?? Quaternion.Identity,
            Scale = scale ?? Vector3.One,
        });

        return entity;
    }

    /// <summary>
    /// 创建带有脚本组件的实体。
    /// </summary>
    public static IEntity CreateScriptEntity(
        IScene scene,
        ulong scriptTypeId,
        string displayName = "ScriptEntity",
        Vector3? position = null)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entity = scene.CreateEntity(displayName);

        entity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        entity.Add(ScriptComponent.Create(scriptTypeId));

        return entity;
    }

    /// <summary>
    /// 创建父子层级结构。
    /// </summary>
    public static (IEntity parent, IEntity child) CreateParentChild(
        IScene scene,
        string parentName = "Parent",
        string childName = "Child",
        Vector3? parentPosition = null,
        Vector3? childLocalPosition = null)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var parent = scene.CreateEntity(parentName);
        parent.Add(new TransformComponent
        {
            Position = parentPosition ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        var child = scene.CreateEntity(childName);
        child.Add(new TransformComponent
        {
            Position = childLocalPosition ?? Vector3.Zero,
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        scene.SetParent(child, parent);

        return (parent, child);
    }

    /// <summary>
    /// 批量创建实体。
    /// </summary>
    public static List<IEntity> CreateBatch(
        IScene scene,
        int count,
        string baseName = "Entity",
        Vector3? startPosition = null,
        Vector3? spacing = null)
    {
        ArgumentNullException.ThrowIfNull(scene);

        var entities = new List<IEntity>(count);
        var start = startPosition ?? Vector3.Zero;
        var step = spacing ?? new Vector3(1, 0, 0);

        for (int i = 0; i < count; i++)
        {
            var entity = scene.CreateEntity($"{baseName}_{i}");
            entity.Add(new TransformComponent
            {
                Position = start + step * i,
                Rotation = Quaternion.Identity,
                Scale = Vector3.One,
            });
            entities.Add(entity);
        }

        return entities;
    }
}
