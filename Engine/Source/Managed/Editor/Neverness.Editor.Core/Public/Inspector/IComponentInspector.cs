using System.Reflection;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Core.Public.Inspector;

/// <summary>
/// 组件检查器排序属性——标注在 Inspector 类上，控制在 Inspector 面板中的显示顺序。
/// 值越小越靠前；未标注时默认 Order = 1000。
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class InspectorOrderAttribute : Attribute
{
    public int Order { get; }
    public InspectorOrderAttribute(int order) => Order = order;
}

/// <summary>
/// 组件检查器接口——定义单个组件类型的读写与 UI 绘制契约。
/// 每种组件类型（Transform、Camera 等）实现此接口，由 <see cref="ComponentInspectorRegistry"/> 统一调度。
/// </summary>
public interface IComponentInspector
{
    /// <summary>组件类型标识（须与 Native 端 FNV-1a hash 一致）。</summary>
    ulong ComponentTypeId { get; }

    /// <summary>组件显示名称（Inspector 折叠标题栏使用）。</summary>
    string DisplayName { get; }

    /// <summary>对应的 C# 结构体 CLR 类型。</summary>
    Type ClrType { get; }

    /// <summary>Inspector 显示优先级，越小越靠前。</summary>
    int Order { get; }

    /// <summary>查询实体是否拥有此组件。</summary>
    bool HasComponent(IEntity entity);

    /// <summary>移除实体的此组件。</summary>
    void RemoveComponent(IEntity entity);

    /// <summary>
    /// 绘制组件的 Inspector UI。
    /// 实现方负责读取组件数据、绘制 ImGui 控件、在用户修改时写回 ECS。
    /// </summary>
    /// <returns>true = 组件数据已被修改（调用方可能需要标记 dirty）。</returns>
    bool DrawInspector(IEntity entity);
}

/// <summary>
/// 泛型组件检查器基类——封装 IEntity 的类型安全读写，
/// 子类只需覆写 <see cref="DrawFields"/> 实现 UI 绘制。
/// </summary>
/// <typeparam name="T">组件结构体类型（须实现 IComponent）。</typeparam>
public abstract class ComponentTypeInspector<T> : IComponentInspector
    where T : struct, IComponent
{
    /// <summary>静态缓存的 TypeId（泛型静态字段，每个 T 只计算一次）。</summary>
    private static readonly ulong s_typeId = ResolveTypeId();

    /// <summary>静态缓存的显示名称。</summary>
    private static readonly string s_displayName = ResolveDisplayName();

    /// <inheritdoc />
    public ulong ComponentTypeId => s_typeId;

    /// <inheritdoc />
    public string DisplayName => s_displayName;

    /// <inheritdoc />
    public Type ClrType => typeof(T);

    /// <inheritdoc />
    public virtual int Order => 1000;

    /// <inheritdoc />
    public bool HasComponent(IEntity entity) =>
        entity.Has<T>();

    /// <inheritdoc />
    public void RemoveComponent(IEntity entity) =>
        entity.Remove<T>();

    /// <inheritdoc />
    public bool DrawInspector(IEntity entity)
    {
        if (!entity.Has<T>())
            return false;

        ref var value = ref entity.Get<T>();
        bool modified = DrawFields(ref value);
        // 修改直接反映到 ECS（ref 返回）

        return modified;
    }

    /// <summary>子类实现：绘制组件字段控件，修改时返回 true。</summary>
    protected abstract bool DrawFields(ref T data);

    /// <summary>从 <see cref="ComponentIdAttribute"/> 提取 TypeId。</summary>
    private static ulong ResolveTypeId()
    {
        var attr = typeof(T).GetCustomAttribute<ComponentIdAttribute>();
        return attr?.TypeId ?? 0;
    }

    /// <summary>从 <see cref="ComponentIdAttribute"/> 提取显示名称，无 attribute 时回退为类型名。</summary>
    private static string ResolveDisplayName()
    {
        var attr = typeof(T).GetCustomAttribute<ComponentIdAttribute>();
        return attr?.Name ?? typeof(T).Name;
    }
}
