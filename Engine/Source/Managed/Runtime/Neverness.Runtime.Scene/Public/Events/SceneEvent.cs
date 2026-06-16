namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景事件类型——与旧 Native NNSceneEventType 对齐。
/// </summary>
public enum SceneEventType : byte
{
    /// <summary>实体被创建。</summary>
    EntityCreated = 0,

    /// <summary>实体被销毁。</summary>
    EntityDestroyed = 1,

    /// <summary>父子关系变更。</summary>
    ParentChanged = 2,

    /// <summary>组件被添加。</summary>
    ComponentAdded = 3,

    /// <summary>组件被移除。</summary>
    ComponentRemoved = 4,

    /// <summary>场景加载完成。</summary>
    SceneLoaded = 5,

    /// <summary>场景卸载。</summary>
    SceneUnloaded = 6,
}

/// <summary>
/// 场景事件——描述实体生命周期和组件变更。
/// 使用 IEntity 接口，不依赖 Native 句柄。
/// </summary>
public readonly struct SceneEvent
{
    /// <summary>事件类型。</summary>
    public SceneEventType Type { get; }

    /// <summary>主实体（EntityCreated / EntityDestroyed / ComponentAdded 的目标实体）。</summary>
    public IEntity? Entity { get; }

    /// <summary>
    /// 附属实体：
    /// - ParentChanged：新父实体
    /// - 其他事件：null
    /// </summary>
    public IEntity? OtherEntity { get; }

    /// <summary>
    /// 组件类型名称：
    /// - ComponentAdded / ComponentRemoved：组件类型名称
    /// - 其他事件：null
    /// </summary>
    public string? ComponentTypeName { get; }

    /// <summary>场景名称（SceneLoaded / SceneUnloaded 用）。</summary>
    public string? SceneName { get; }

    public SceneEvent(SceneEventType type, IEntity? entity = null,
        IEntity? otherEntity = null, string? componentTypeName = null, string? sceneName = null)
    {
        Type = type;
        Entity = entity;
        OtherEntity = otherEntity;
        ComponentTypeName = componentTypeName;
        SceneName = sceneName;
    }

    /// <summary>创建实体创建事件。</summary>
    public static SceneEvent OnEntityCreated(IEntity entity) =>
        new(SceneEventType.EntityCreated, entity);

    /// <summary>创建实体销毁事件。</summary>
    public static SceneEvent OnEntityDestroyed(IEntity entity) =>
        new(SceneEventType.EntityDestroyed, entity);

    /// <summary>创建父子关系变更事件。</summary>
    public static SceneEvent OnParentChanged(IEntity entity, IEntity newParent) =>
        new(SceneEventType.ParentChanged, entity, newParent);

    /// <summary>创建组件添加事件。</summary>
    public static SceneEvent OnComponentAdded(IEntity entity, string componentTypeName) =>
        new(SceneEventType.ComponentAdded, entity, componentTypeName: componentTypeName);

    /// <summary>创建组件移除事件。</summary>
    public static SceneEvent OnComponentRemoved(IEntity entity, string componentTypeName) =>
        new(SceneEventType.ComponentRemoved, entity, componentTypeName: componentTypeName);

    /// <summary>创建场景加载事件。</summary>
    public static SceneEvent OnSceneLoaded(string sceneName) =>
        new(SceneEventType.SceneLoaded, sceneName: sceneName);

    /// <summary>创建场景卸载事件。</summary>
    public static SceneEvent OnSceneUnloaded(string sceneName) =>
        new(SceneEventType.SceneUnloaded, sceneName: sceneName);
}
