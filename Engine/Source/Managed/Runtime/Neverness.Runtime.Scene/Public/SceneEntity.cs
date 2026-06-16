using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景实体——IEntity 的便捷包装。
/// 提供与旧 API 兼容的属性和方法。
/// </summary>
public sealed class SceneEntity : IEntity
{
    private readonly IEntity _entity;
    private readonly IScene _scene;

    /// <summary>实体 ID。</summary>
    public int Id => _entity.Id;

    /// <summary>IEntity 接口：实体名称。</summary>
    string? IEntity.Name
    {
        get => _entity.Name;
        set => _entity.Name = value;
    }

    /// <summary>IEntity 接口：是否有效。</summary>
    bool IEntity.IsValid => _entity.IsValid;

    /// <summary>旧 API 兼容：实体句柄（使用 ID 代替）。</summary>
    public NNEntityHandle Handle => new((ulong)Id);

    /// <summary>旧 API 兼容：实体值（返回 ID）。</summary>
    public ulong Value => (ulong)Id;

    /// <summary>显示名称。</summary>
    public string DisplayName
    {
        get => _entity.Name ?? $"Entity_{_entity.Id}";
        set => _entity.Name = value;
    }

    /// <summary>是否存活。</summary>
    public bool IsAlive => _entity.IsValid;

    /// <summary>底层 IEntity 接口。</summary>
    public IEntity Entity => _entity;

    /// <summary>所属场景。</summary>
    public IScene Scene => _scene;

    public SceneEntity(IEntity entity, IScene scene)
    {
        _entity = entity ?? throw new ArgumentNullException(nameof(entity));
        _scene = scene ?? throw new ArgumentNullException(nameof(scene));
    }

    // ── 组件操作 ──

    /// <summary>IEntity 接口：添加组件。</summary>
    void IEntity.Add<T>(T component) => _entity.Add(component);

    /// <summary>IEntity 接口：移除组件。</summary>
    void IEntity.Remove<T>() => _entity.Remove<T>();

    /// <summary>IEntity 接口：获取组件引用。</summary>
    ref T IEntity.Get<T>() => ref _entity.Get<T>();

    /// <summary>IEntity 接口：检查是否拥有组件。</summary>
    bool IEntity.Has<T>() => _entity.Has<T>();

    /// <summary>IEntity 接口：尝试获取组件。</summary>
    bool IEntity.TryGet<T>(out T component) => _entity.TryGet(out component);

    /// <summary>添加组件。</summary>
    public void AddComponent<T>(T component) where T : struct, IComponent
    {
        _entity.Add(component);
    }

    /// <summary>旧 API 兼容：添加组件（无参数版本）。</summary>
    public void AddComponent<T>() where T : struct, IComponent
    {
        _entity.Add(default(T));
    }

    /// <summary>旧 API 兼容：设置组件（等同于 AddComponent）。</summary>
    public void SetComponent<T>(T component) where T : struct, IComponent
    {
        _entity.Add(component);
    }

    /// <summary>移除组件。</summary>
    public void RemoveComponent<T>() where T : struct, IComponent
    {
        _entity.Remove<T>();
    }

    /// <summary>查询是否拥有组件。</summary>
    public bool HasComponent<T>() where T : struct, IComponent
    {
        return _entity.Has<T>();
    }

    /// <summary>读取组件数据。</summary>
    public ref T GetComponent<T>() where T : struct, IComponent
    {
        return ref _entity.Get<T>();
    }

    /// <summary>尝试获取组件。</summary>
    public bool TryGetComponent<T>(out T component) where T : struct, IComponent
    {
        return _entity.TryGet(out component);
    }

    // ── 标签操作 ──

    /// <summary>IEntity 接口：添加标签。</summary>
    void IEntity.AddTag<T>() => _entity.AddTag<T>();

    /// <summary>IEntity 接口：移除标签。</summary>
    void IEntity.RemoveTag<T>() => _entity.RemoveTag<T>();

    /// <summary>IEntity 接口：检查标签。</summary>
    bool IEntity.HasTag<T>() => _entity.HasTag<T>();

    /// <summary>添加标签。</summary>
    public void AddTag<T>() where T : struct, ITag
    {
        _entity.AddTag<T>();
    }

    /// <summary>移除标签。</summary>
    public void RemoveTag<T>() where T : struct, ITag
    {
        _entity.RemoveTag<T>();
    }

    /// <summary>检查标签。</summary>
    public bool HasTag<T>() where T : struct, ITag
    {
        return _entity.HasTag<T>();
    }

    // ── 层级操作 ──

    /// <summary>设置父实体。</summary>
    public void SetParent(SceneEntity parent)
    {
        _scene.SetParent(_entity, parent._entity);
    }

    /// <summary>获取父实体。</summary>
    public SceneEntity? GetParent()
    {
        var parent = _scene.GetParent(_entity);
        if (parent == null || !parent.IsValid) return null;
        return new SceneEntity(parent, _scene);
    }

    /// <summary>获取子实体列表。</summary>
    public IReadOnlyList<SceneEntity> GetChildren()
    {
        var children = _scene.GetChildren(_entity);
        return children.Select(c => new SceneEntity(c, _scene)).ToList();
    }

    // ── 生命周期 ──

    /// <summary>IEntity 接口：销毁实体。</summary>
    void IEntity.Destroy() => _scene.DestroyEntity(_entity);

    /// <summary>销毁实体。</summary>
    public void Destroy()
    {
        _scene.DestroyEntity(_entity);
    }

    // ── 相等性 ──

    public override bool Equals(object? obj)
    {
        if (obj is SceneEntity other)
            return _entity.Id == other._entity.Id;
        return false;
    }

    public override int GetHashCode() => _entity.Id;

    public static bool operator ==(SceneEntity? left, SceneEntity? right)
    {
        if (left is null) return right is null;
        return left.Equals(right);
    }

    public static bool operator !=(SceneEntity? left, SceneEntity? right) => !(left == right);
}
