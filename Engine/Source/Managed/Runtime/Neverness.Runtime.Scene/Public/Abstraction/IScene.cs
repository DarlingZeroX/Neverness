using Neverness.Runtime.Scene.Query;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景抽象接口——与 ECS 实现无关。
/// 上层代码（Editor、Gameplay、Render）只依赖此接口。
/// </summary>
public interface IScene : IDisposable
{
    /// <summary>场景名称。</summary>
    string Name { get; set; }

    /// <summary>实体数量。</summary>
    int EntityCount { get; }

    /// <summary>是否有效（未释放）。</summary>
    bool IsValid { get; }

    /// <summary>事件总线。</summary>
    ISceneEventBus Events { get; }

    // ── 实体操作 ──

    /// <summary>创建实体。</summary>
    IEntity CreateEntity(string? displayName = null);

    /// <summary>销毁实体。</summary>
    void DestroyEntity(IEntity entity);

    /// <summary>销毁所有实体。</summary>
    void DestroyAllEntities();

    /// <summary>获取实体。如果不存在返回 null。</summary>
    IEntity? GetEntity(int entityId);

    /// <summary>检查实体是否存在。</summary>
    bool EntityExists(int entityId);

    // ── 查询 ──

    /// <summary>创建单组件查询。</summary>
    ISceneQuery<T1> Query<T1>()
        where T1 : struct, IComponent;

    /// <summary>创建双组件查询。</summary>
    ISceneQuery<T1, T2> Query<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent;

    /// <summary>创建三组件查询。</summary>
    ISceneQuery<T1, T2, T3> Query<T1, T2, T3>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
        where T3 : struct, IComponent;

    // ── 视图 ──

    /// <summary>创建单组件视图。</summary>
    SceneView<T> CreateView<T>() where T : struct, IComponent;

    /// <summary>创建双组件视图。</summary>
    SceneView<T1, T2> CreateView<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent;

    // ── 系统管理 ──

    /// <summary>添加系统。</summary>
    void AddSystem(ISceneSystem system);

    /// <summary>移除系统。</summary>
    void RemoveSystem(ISceneSystem system);

    /// <summary>获取系统。</summary>
    T? GetSystem<T>() where T : class;

    // ── 更新 ──

    /// <summary>主帧更新——驱动所有系统。</summary>
    void Update(float deltaTime);

    /// <summary>固定步长更新——驱动物理等固定步长系统。</summary>
    void FixedUpdate(float fixedDeltaTime);

    /// <summary>按标签掩码更新——仅更新匹配标签的系统。</summary>
    void UpdateByTagMask(float deltaTime, SceneSystemTags mask);

    // ── 层级操作 ──

    /// <summary>设置父子关系。</summary>
    void SetParent(IEntity child, IEntity parent);

    /// <summary>获取父实体。</summary>
    IEntity? GetParent(IEntity entity);

    /// <summary>获取子实体列表。</summary>
    IReadOnlyList<IEntity> GetChildren(IEntity entity);

    // ── 序列化 ──

    /// <summary>序列化场景到流。</summary>
    void Serialize(Stream stream, string format = "json");

    /// <summary>从流反序列化场景。</summary>
    void Deserialize(Stream stream, string format = "json");
}
