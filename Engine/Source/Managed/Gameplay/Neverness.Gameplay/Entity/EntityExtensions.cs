// ============================================================================
// EntityExtensions.cs - Entity 扩展方法
// ============================================================================
// Entity 的扩展方法集合。
// ============================================================================

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
    /// 获取或添加组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>组件值。</returns>
    public static T GetOrAddComponent<T>(this Entity entity) where T : struct, new()
    {
        var result = entity.GetComponent<T>();
        if (result.HasValue)
        {
            return result.Value;
        }
        return entity.AddComponent<T>();
    }

    /// <summary>
    /// 安全获取组件（无组件时返回默认值）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>组件值或默认值。</returns>
    public static T GetComponentOrDefault<T>(this Entity entity) where T : struct
    {
        return entity.GetComponent<T>() ?? default;
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
