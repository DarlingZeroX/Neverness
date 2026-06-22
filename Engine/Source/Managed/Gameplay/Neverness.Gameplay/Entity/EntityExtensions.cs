// ============================================================================
// EntityExtensions.cs - Entity 扩展方法
// ============================================================================
// Entity 的扩展方法集合。
// ============================================================================

using Neverness.Runtime.Scene;

namespace Neverness.Gameplay;

/// <summary>
/// Entity 扩展方法。
/// </summary>
public static class EntityExtensions
{
    // ========================================================================
    // 组件扩展
    // ========================================================================

    /// <summary>
    /// 获取或添加组件（不存在时自动添加默认值）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>组件值拷贝。</returns>
    /// <remarks>
    /// ⚠️ 返回的是值拷贝。如需修改已存在的组件，请使用 ref GetComponent：
    /// <code>
    /// // 需要修改时：先确保存在，再拿 ref
    /// if (!entity.HasComponent&lt;TransformComponent&gt;())
    ///     entity.AddComponent&lt;TransformComponent&gt;();
    /// ref var transform = ref entity.GetComponent&lt;TransformComponent&gt;();
    /// transform.Position.X = 10;  // 直接生效
    /// </code>
    /// </remarks>
    public static T GetOrAddComponent<T>(this Entity entity) where T : struct, IComponent
    {
        if (entity.TryGetComponent<T>(out var component))
        {
            return component;
        }
        return entity.AddComponent<T>();
    }

    /// <summary>
    /// 安全获取组件快照（无组件时返回默认值，适用于只读场景）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>组件值拷贝或默认值。</returns>
    /// <remarks>
    /// ⚠️ 返回的是值拷贝，修改不会反映到 ECS。
    /// 如需修改组件，请使用 HasComponent + GetComponent 组合。
    /// </remarks>
    public static T GetComponentOrDefault<T>(this Entity entity) where T : struct, IComponent
    {
        if (entity.TryGetComponent<T>(out var component))
        {
            return component;
        }
        return default;
    }

    // ========================================================================
    // Behaviour 扩展
    // ========================================================================

    /// <summary>
    /// 获取或添加 Behaviour。
    /// </summary>
    /// <typeparam name="T">Behaviour 类型。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>Behaviour 实例。</returns>
    public static T GetOrAddBehaviour<T>(this Entity entity) where T : EntityBehaviour, new()
    {
        var existing = entity.GetBehaviour<T>();
        if (existing != null)
        {
            return existing;
        }
        return entity.AddBehaviour<T>();
    }

    /// <summary>
    /// 检查是否拥有指定类型的 Behaviour。
    /// </summary>
    /// <typeparam name="T">Behaviour 类型。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>是否拥有指定类型的 Behaviour。</returns>
    public static bool HasBehaviour<T>(this Entity entity) where T : EntityBehaviour
    {
        return entity.GetBehaviour<T>() != null;
    }

    // ========================================================================
    // 层级扩展
    // ========================================================================

    /// <summary>
    /// 获取所有子实体。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>子实体列表。</returns>
    public static IReadOnlyList<Entity> GetChildren(this Entity entity)
    {
        // TODO: 实现子实体查询
        return Array.Empty<Entity>();
    }

    /// <summary>
    /// 查找子实体（按名称）。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <param name="name">子实体名称。</param>
    /// <returns>找到的子实体，未找到时返回 null。</returns>
    public static Entity? FindChild(this Entity entity, string name)
    {
        var children = entity.GetChildren();
        return children.FirstOrDefault(c => c.Name == name);
    }
}
