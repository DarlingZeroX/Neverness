using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景事件类型——与 Native <c>NNSceneEventType</c> 对齐。
/// </summary>
public enum SceneEventType : byte
{
    /// <summary>实体被创建。</summary>
    EntityCreated = 0,

    /// <summary>实体被销毁。</summary>
    EntityDestroyed = 1,

    /// <summary>父子关系变更。</summary>
    ParentChanged = 2,

    /// <summary>组件被写入（Emplace / SetComponent）。</summary>
    ComponentEmplaced = 3,
}

/// <summary>
/// 场景事件——描述实体生命周期和组件变更。
/// 对应 Native <c>NNSceneEntityEvent</c> 结构体。
/// </summary>
public readonly struct SceneEvent
{
    /// <summary>事件类型。</summary>
    public SceneEventType Type { get; }

    /// <summary>主实体句柄（EntityCreated / EntityDestroyed / ComponentEmplaced 的目标实体）。</summary>
    public NNEntityHandle Entity { get; }

    /// <summary>
    /// 附属实体句柄：
    /// - ParentChanged：新父实体
    /// - 其他事件：零句柄
    /// </summary>
    public NNEntityHandle OtherEntity { get; }

    /// <summary>
    /// 组件类型 ID：
    /// - ComponentEmplaced：被写入的组件 TypeId
    /// - 其他事件：0
    /// </summary>
    public ulong ComponentTypeId { get; }

    public SceneEvent(SceneEventType type, NNEntityHandle entity,
        NNEntityHandle otherEntity = default, ulong componentTypeId = 0)
    {
        Type = type;
        Entity = entity;
        OtherEntity = otherEntity;
        ComponentTypeId = componentTypeId;
    }

    /// <summary>创建实体创建事件。</summary>
    public static SceneEvent OnEntityCreated(NNEntityHandle entity) =>
        new(SceneEventType.EntityCreated, entity);

    /// <summary>创建实体销毁事件。</summary>
    public static SceneEvent OnEntityDestroyed(NNEntityHandle entity) =>
        new(SceneEventType.EntityDestroyed, entity);

    /// <summary>创建父子关系变更事件。</summary>
    public static SceneEvent OnParentChanged(NNEntityHandle entity, NNEntityHandle newParent) =>
        new(SceneEventType.ParentChanged, entity, newParent);

    /// <summary>创建组件写入事件。</summary>
    public static SceneEvent OnComponentEmplaced(NNEntityHandle entity, ulong componentTypeId) =>
        new(SceneEventType.ComponentEmplaced, entity, componentTypeId: componentTypeId);
}
