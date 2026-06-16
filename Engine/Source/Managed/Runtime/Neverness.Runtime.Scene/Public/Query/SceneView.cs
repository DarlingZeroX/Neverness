using Friflo.Engine.ECS;

namespace Neverness.Runtime.Scene.Query;

/// <summary>
/// 场景视图——提供高效的批量查询结果访问。
/// 封装 Friflo 的查询结果，提供类型安全的访问接口。
/// </summary>
public sealed class SceneView<T> where T : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly IScene _scene;
    private readonly List<IEntity> _entities = new();
    private readonly List<T> _components = new();

    internal SceneView(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>匹配的实体数量。</summary>
    public int Count => _entities.Count;

    /// <summary>获取指定索引的实体。</summary>
    public IEntity GetEntity(int index) => _entities[index];

    /// <summary>获取指定索引的组件值。</summary>
    public T GetComponent(int index) => _components[index];

    /// <summary>执行查询并填充视图。</summary>
    public void Refresh()
    {
        _entities.Clear();
        _components.Clear();

        _store.Query<T>().ForEachEntity((ref T component, Entity entity) =>
        {
            _entities.Add(new Internal.FrifloEntity(entity, (Internal.FrifloScene)_scene));
            _components.Add(component);
        });
    }

    /// <summary>遍历所有匹配实体。</summary>
    public void ForEach(Action<IEntity, T> action)
    {
        for (int i = 0; i < Count; i++)
        {
            action(_entities[i], _components[i]);
        }
    }

    /// <summary>获取所有实体。</summary>
    public IReadOnlyList<IEntity> GetEntities() => _entities;

    /// <summary>获取所有组件。</summary>
    public IReadOnlyList<T> GetComponents() => _components;
}

/// <summary>
/// 双组件场景视图。
/// </summary>
public sealed class SceneView<T1, T2>
    where T1 : struct, IComponent
    where T2 : struct, IComponent
{
    private readonly EntityStore _store;
    private readonly IScene _scene;
    private readonly List<IEntity> _entities = new();
    private readonly List<T1> _components1 = new();
    private readonly List<T2> _components2 = new();

    internal SceneView(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>匹配的实体数量。</summary>
    public int Count => _entities.Count;

    /// <summary>获取指定索引的实体。</summary>
    public IEntity GetEntity(int index) => _entities[index];

    /// <summary>获取指定索引的第一个组件。</summary>
    public T1 GetComponent1(int index) => _components1[index];

    /// <summary>获取指定索引的第二个组件。</summary>
    public T2 GetComponent2(int index) => _components2[index];

    /// <summary>执行查询并填充视图。</summary>
    public void Refresh()
    {
        _entities.Clear();
        _components1.Clear();
        _components2.Clear();

        _store.Query<T1, T2>().ForEachEntity((ref T1 c1, ref T2 c2, Entity entity) =>
        {
            _entities.Add(new Internal.FrifloEntity(entity, (Internal.FrifloScene)_scene));
            _components1.Add(c1);
            _components2.Add(c2);
        });
    }

    /// <summary>遍历所有匹配实体。</summary>
    public void ForEach(Action<IEntity, T1, T2> action)
    {
        for (int i = 0; i < Count; i++)
        {
            action(_entities[i], _components1[i], _components2[i]);
        }
    }
}
