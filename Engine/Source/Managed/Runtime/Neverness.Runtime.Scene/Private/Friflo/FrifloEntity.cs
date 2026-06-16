using Friflo.Engine.ECS;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// 实体名称标签——用于存储实体的显示名称。
/// </summary>
public struct EntityName : ITag
{
    public string value;
}

/// <summary>
/// IEntity 的 Friflo ECS 实现。
/// 包装 Friflo.Entity，对外只暴露 IEntity 接口。
/// </summary>
internal sealed class FrifloEntity : IEntity
{
    private readonly Entity _entity;
    private readonly FrifloScene _scene;

    // ── IEntity 属性 ──

    public int Id => _entity.Id;

    public string? Name
    {
        get
        {
            if (_entity.HasComponent<TagComponent>())
            {
                return _entity.GetComponent<TagComponent>().Name;
            }
            return $"Entity_{_entity.Id}";
        }
        set
        {
            if (value == null) return;
            if (_entity.HasComponent<TagComponent>())
            {
                ref var tag = ref _entity.GetComponent<TagComponent>();
                tag.Name = value;
            }
            else
            {
                _entity.AddComponent(TagComponent.Create(value));
            }
        }
    }

    public bool IsValid => !_entity.IsNull;

    // ── 构造 ──

    internal FrifloEntity(Entity entity, FrifloScene scene)
    {
        _entity = entity;
        _scene = scene;
    }

    internal Entity InternalEntity => _entity;

    // ── 组件操作 ──

    public void Add<T>(T component) where T : struct, IComponent
    {
        _entity.AddComponent(component);
    }

    public void Remove<T>() where T : struct, IComponent
    {
        _entity.RemoveComponent<T>();
    }

    public ref T Get<T>() where T : struct, IComponent
    {
        return ref _entity.GetComponent<T>();
    }

    public bool Has<T>() where T : struct, IComponent
    {
        return _entity.HasComponent<T>();
    }

    public bool TryGet<T>(out T component) where T : struct, IComponent
    {
        if (_entity.HasComponent<T>())
        {
            component = _entity.GetComponent<T>();
            return true;
        }
        component = default;
        return false;
    }

    // ── 标签操作（暂时简化）──

    public void AddTag<T>() where T : struct, ITag
    {
        _entity.AddTag<T>();
    }

    public void RemoveTag<T>() where T : struct, ITag
    {
        _entity.RemoveTag<T>();
    }

    public bool HasTag<T>() where T : struct, ITag
    {
        // Friflo 的 HasTag API 需要确认
        // 暂时返回 false
        return false;
    }

    // ── 生命周期 ──

    public void Destroy()
    {
        _entity.DeleteEntity();
    }

    // ── 相等性 ──

    public override bool Equals(object? obj)
    {
        if (obj is FrifloEntity other)
            return _entity.Id == other._entity.Id;
        return false;
    }

    public override int GetHashCode() => _entity.Id;

    public static bool operator ==(FrifloEntity? left, FrifloEntity? right)
    {
        if (left is null) return right is null;
        return left.Equals(right);
    }

    public static bool operator !=(FrifloEntity? left, FrifloEntity? right) => !(left == right);
}
