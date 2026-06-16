using Friflo.Engine.ECS;
using Neverness.Runtime.Scene.Query;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// ISceneQuery 的 Friflo ECS 实现（单组件）。
/// </summary>
internal sealed class FrifloQuery<T1> : ISceneQuery<T1>
    where T1 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly FrifloScene _scene;
    private ArchetypeQuery<T1>? _query;

    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    public FrifloQuery(EntityStore store, FrifloScene scene)
    {
        _store = store;
        _scene = scene;
    }

    public void ForEach(ForEachAction<T1> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T1 c1, Entity entity) =>
        {
            var wrappedEntity = new FrifloEntity(entity, _scene);
            action(ref c1, wrappedEntity);
        });
    }

    public IEntity? FirstOrDefault()
    {
        EnsureQuery();
        Entity result = default;
        _query!.ForEachEntity((ref T1 c1, Entity entity) =>
        {
            if (result.IsNull)
                result = entity;
        });
        if (result.IsNull)
            return null;
        return new FrifloEntity(result, _scene);
    }

    public bool Any()
    {
        EnsureQuery();
        return _query!.Count > 0;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T1>();
    }
}

/// <summary>
/// ISceneQuery 的 Friflo ECS 实现（双组件）。
/// </summary>
internal sealed class FrifloQuery<T1, T2> : ISceneQuery<T1, T2>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly FrifloScene _scene;
    private ArchetypeQuery<T1, T2>? _query;

    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    public FrifloQuery(EntityStore store, FrifloScene scene)
    {
        _store = store;
        _scene = scene;
    }

    public void ForEach(ForEachAction<T1, T2> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, Entity entity) =>
        {
            var wrappedEntity = new FrifloEntity(entity, _scene);
            action(ref c1, ref c2, wrappedEntity);
        });
    }

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
        return new FrifloEntity(result, _scene);
    }

    public bool Any()
    {
        EnsureQuery();
        return _query!.Count > 0;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T1, T2>();
    }
}

/// <summary>
/// ISceneQuery 的 Friflo ECS 实现（三组件）。
/// </summary>
internal sealed class FrifloQuery<T1, T2, T3> : ISceneQuery<T1, T2, T3>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
    where T3 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly FrifloScene _scene;
    private ArchetypeQuery<T1, T2, T3>? _query;

    public int Count
    {
        get
        {
            EnsureQuery();
            return _query!.Count;
        }
    }

    public FrifloQuery(EntityStore store, FrifloScene scene)
    {
        _store = store;
        _scene = scene;
    }

    public void ForEach(ForEachAction<T1, T2, T3> action)
    {
        EnsureQuery();
        _query!.ForEachEntity((ref T1 c1, ref T2 c2, ref T3 c3, Entity entity) =>
        {
            var wrappedEntity = new FrifloEntity(entity, _scene);
            action(ref c1, ref c2, ref c3, wrappedEntity);
        });
    }

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
        return new FrifloEntity(result, _scene);
    }

    public bool Any()
    {
        EnsureQuery();
        return _query!.Count > 0;
    }

    private void EnsureQuery()
    {
        _query ??= _store.Query<T1, T2, T3>();
    }
}
