using Friflo.Engine.ECS;

namespace Neverness.Runtime.Scene.Query;

// ── 委托定义 ──

/// <summary>单组件遍历委托。</summary>
public delegate void ForEachAction<T1>(ref T1 component, IEntity entity)
    where T1 : struct, IComponent;

/// <summary>双组件遍历委托。</summary>
public delegate void ForEachAction<T1, T2>(ref T1 c1, ref T2 c2, IEntity entity)
    where T1 : struct, IComponent
    where T2 : struct, IComponent;

/// <summary>三组件遍历委托。</summary>
public delegate void ForEachAction<T1, T2, T3>(ref T1 c1, ref T2 c2, ref T3 c3, IEntity entity)
    where T1 : struct, IComponent
    where T2 : struct, IComponent
    where T3 : struct, IComponent;

/// <summary>
/// 单组件查询接口——与 ECS 实现无关。
/// </summary>
public interface ISceneQuery<T1>
    where T1 : struct, IComponent
{
    int Count { get; }
    void ForEach(ForEachAction<T1> action);
    IEntity? FirstOrDefault();
    bool Any();
}

/// <summary>
/// 双组件查询接口——与 ECS 实现无关。
/// </summary>
public interface ISceneQuery<T1, T2>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
{
    int Count { get; }
    void ForEach(ForEachAction<T1, T2> action);
    IEntity? FirstOrDefault();
    bool Any();
}

/// <summary>
/// 三组件查询接口——与 ECS 实现无关。
/// </summary>
public interface ISceneQuery<T1, T2, T3>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
    where T3 : struct, IComponent
{
    int Count { get; }
    void ForEach(ForEachAction<T1, T2, T3> action);
    IEntity? FirstOrDefault();
    bool Any();
}

/// <summary>
/// 单组件缓存式查询——封装 Friflo ArchetypeQuery，提供类型安全的查询接口。
/// 由 <see cref="SceneQueryCache"/> 管理，可复用避免重复分配。
/// </summary>
public sealed class SceneQuery<T> where T : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly IScene _scene;
    private ArchetypeQuery<T>? _query;

    internal SceneQuery(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>获取拥有指定组件的实体数量。</summary>
    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    /// <summary>执行查询并遍历所有匹配实体。</summary>
    public void ForEach(ForEachAction<T> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T component, Entity entity) =>
        {
            var wrappedEntity = new Internal.FrifloEntity(entity, (Internal.FrifloScene)_scene);
            action(ref component, wrappedEntity);
        });
    }

    /// <summary>获取第一个匹配实体。如果没有则返回 null。</summary>
    public IEntity? FirstOrDefault()
    {
        EnsureQuery();
        Entity result = default;
        _query!.ForEachEntity((ref T component, Entity entity) =>
        {
            if (result.IsNull)
                result = entity;
        });
        if (result.IsNull)
            return null;
        return new Internal.FrifloEntity(result, (Internal.FrifloScene)_scene);
    }

    /// <summary>检查是否有匹配实体。</summary>
    public bool Any()
    {
        return Count > 0;
    }

    /// <summary>获取所有匹配实体的列表。</summary>
    public List<IEntity> ToList()
    {
        var list = new List<IEntity>();
        ForEach((ref T component, IEntity entity) =>
        {
            list.Add(entity);
        });
        return list;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T>();
    }
}

/// <summary>
/// 双组件缓存式查询。
/// </summary>
public sealed class SceneQuery<T1, T2>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly IScene _scene;
    private ArchetypeQuery<T1, T2>? _query;

    internal SceneQuery(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>获取同时拥有两个组件的实体数量。</summary>
    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    /// <summary>执行查询并遍历所有匹配实体。</summary>
    public void ForEach(ForEachAction<T1, T2> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, Entity entity) =>
        {
            var wrappedEntity = new Internal.FrifloEntity(entity, (Internal.FrifloScene)_scene);
            action(ref c1, ref c2, wrappedEntity);
        });
    }

    /// <summary>获取第一个匹配实体。如果没有则返回 null。</summary>
    public IEntity? FirstOrDefault()
    {
        EnsureQuery();
        Entity result = default;
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, Entity entity) =>
        {
            if (result.IsNull)
                result = entity;
        });
        if (result.IsNull)
            return null;
        return new Internal.FrifloEntity(result, (Internal.FrifloScene)_scene);
    }

    /// <summary>检查是否有匹配实体。</summary>
    public bool Any()
    {
        return Count > 0;
    }

    /// <summary>获取所有匹配实体的列表。</summary>
    public List<IEntity> ToList()
    {
        var list = new List<IEntity>();
        ForEach((ref T1 c1, ref T2 c2, IEntity entity) =>
        {
            list.Add(entity);
        });
        return list;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T1, T2>();
    }
}

/// <summary>
/// 三组件缓存式查询。
/// </summary>
public sealed class SceneQuery<T1, T2, T3>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
    where T3 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly IScene _scene;
    private ArchetypeQuery<T1, T2, T3>? _query;

    internal SceneQuery(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>获取同时拥有三个组件的实体数量。</summary>
    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    /// <summary>执行查询并遍历所有匹配实体。</summary>
    public void ForEach(ForEachAction<T1, T2, T3> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, ref T3 c3, Entity entity) =>
        {
            var wrappedEntity = new Internal.FrifloEntity(entity, (Internal.FrifloScene)_scene);
            action(ref c1, ref c2, ref c3, wrappedEntity);
        });
    }

    /// <summary>获取第一个匹配实体。如果没有则返回 null。</summary>
    public IEntity? FirstOrDefault()
    {
        EnsureQuery();
        Entity result = default;
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, ref T3 c3, Entity entity) =>
        {
            if (result.IsNull)
                result = entity;
        });
        if (result.IsNull)
            return null;
        return new Internal.FrifloEntity(result, (Internal.FrifloScene)_scene);
    }

    /// <summary>检查是否有匹配实体。</summary>
    public bool Any()
    {
        return Count > 0;
    }

    /// <summary>获取所有匹配实体的列表。</summary>
    public List<IEntity> ToList()
    {
        var list = new List<IEntity>();
        ForEach((ref T1 c1, ref T2 c2, ref T3 c3, IEntity entity) =>
        {
            list.Add(entity);
        });
        return list;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T1, T2, T3>();
    }
}
