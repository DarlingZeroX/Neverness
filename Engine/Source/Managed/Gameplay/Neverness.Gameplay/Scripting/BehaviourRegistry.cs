// ============================================================================
// BehaviourRegistry.cs - Entity ↔ Behaviour 映射注册表
// ============================================================================
// 管理 Entity ↔ Behaviour 的双向映射。
// 由 ScriptBehaviourScheduler 独立持有，不存储在 ECS 中。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 脚本行为注册表：管理 Entity ↔ Behaviour 的双向映射。
/// </summary>
/// <remarks>
/// ⚠️ 由 ScriptBehaviourScheduler 独立持有，不存储在 ECS 中。
///
/// 映射关系：
/// - Entity (ulong handle) → List&lt;EntityBehaviour&gt;
/// - EntityBehaviour → Entity (ulong handle)
/// </remarks>
public sealed class BehaviourRegistry
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>Entity Handle → Behaviour 列表。</summary>
    private readonly Dictionary<ulong, List<EntityBehaviour>> _entityToBehaviours = new();

    /// <summary>Behaviour 实例 → Entity Handle。</summary>
    private readonly Dictionary<EntityBehaviour, ulong> _behaviourToEntity = new();

    /// <summary>所有活跃的 Behaviour（用于遍历）。</summary>
    private readonly List<EntityBehaviour> _allBehaviours = new();

    /// <summary>待销毁队列。</summary>
    private readonly Queue<EntityBehaviour> _pendingDestroy = new();

    // ========================================================================
    // 公共属性
    // ========================================================================

    /// <summary>所有活跃的 Behaviour 数量。</summary>
    public int Count => _allBehaviours.Count;

    /// <summary>所有活跃的 Behaviour（只读）。</summary>
    public IReadOnlyList<EntityBehaviour> AllBehaviours => _allBehaviours;

    // ========================================================================
    // 注册方法
    // ========================================================================

    /// <summary>
    /// 注册 Behaviour。
    /// </summary>
    /// <param name="entityHandle">Entity 句柄。</param>
    /// <param name="behaviour">Behaviour 实例。</param>
    public void Register(ulong entityHandle, EntityBehaviour behaviour)
    {
        ArgumentNullException.ThrowIfNull(behaviour);

        // 获取或创建 Entity 的 Behaviour 列表
        if (!_entityToBehaviours.TryGetValue(entityHandle, out var list))
        {
            list = new List<EntityBehaviour>();
            _entityToBehaviours[entityHandle] = list;
        }

        // 检查是否已注册
        if (list.Contains(behaviour))
        {
            return;
        }

        // 注册
        list.Add(behaviour);
        _behaviourToEntity[behaviour] = entityHandle;
        _allBehaviours.Add(behaviour);
    }

    // ========================================================================
    // 查询方法
    // ========================================================================

    /// <summary>
    /// 获取 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>Behaviour 列表。</returns>
    public IReadOnlyList<EntityBehaviour> GetBehaviours(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        return GetBehaviours(entity.Id);
    }

    /// <summary>
    /// 获取 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entityHandle">Entity 句柄。</param>
    /// <returns>Behaviour 列表。</returns>
    public IReadOnlyList<EntityBehaviour> GetBehaviours(ulong entityHandle)
    {
        return _entityToBehaviours.TryGetValue(entityHandle, out var list)
            ? list
            : Array.Empty<EntityBehaviour>();
    }

    /// <summary>
    /// 获取 Behaviour 所属的 Entity 句柄。
    /// </summary>
    /// <param name="behaviour">Behaviour 实例。</param>
    /// <param name="entityHandle">输出的 Entity 句柄。</param>
    /// <returns>是否找到。</returns>
    public bool TryGetEntityHandle(EntityBehaviour behaviour, out ulong entityHandle)
    {
        return _behaviourToEntity.TryGetValue(behaviour, out entityHandle);
    }

    /// <summary>
    /// 检查 Entity 是否有指定类型的 Behaviour。
    /// </summary>
    /// <typeparam name="T">Behaviour 类型。</typeparam>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>是否有指定类型的 Behaviour。</returns>
    public bool HasBehaviour<T>(Entity entity) where T : EntityBehaviour
    {
        var behaviours = GetBehaviours(entity);
        return behaviours.Any(b => b is T);
    }

    // ========================================================================
    // 销毁方法
    // ========================================================================

    /// <summary>
    /// 标记 Behaviour 待销毁。
    /// </summary>
    /// <param name="behaviour">Behaviour 实例。</param>
    public void MarkForDestroy(EntityBehaviour behaviour)
    {
        ArgumentNullException.ThrowIfNull(behaviour);

        if (behaviour.IsDestroyed)
        {
            return;
        }

        behaviour.IsDestroyed = true;
        _pendingDestroy.Enqueue(behaviour);
    }

    /// <summary>
    /// 销毁 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    public void DestroyAllBehaviours(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        DestroyAllBehaviours(entity.Id);
    }

    /// <summary>
    /// 销毁 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entityHandle">Entity 句柄。</param>
    public void DestroyAllBehaviours(ulong entityHandle)
    {
        if (!_entityToBehaviours.TryGetValue(entityHandle, out var list))
        {
            return;
        }

        // 标记所有 Behaviour 待销毁
        foreach (var behaviour in list)
        {
            MarkForDestroy(behaviour);
        }
    }

    /// <summary>
    /// 处理待销毁队列。
    /// </summary>
    /// <returns>已销毁的 Behaviour 数量。</returns>
    public int ProcessPendingDestroy()
    {
        int count = 0;

        while (_pendingDestroy.Count > 0)
        {
            var behaviour = _pendingDestroy.Dequeue();

            // 调用 OnDestroy
            behaviour.OnDestroy();

            // 从映射中移除
            if (_behaviourToEntity.TryGetValue(behaviour, out var entityHandle))
            {
                _behaviourToEntity.Remove(behaviour);

                if (_entityToBehaviours.TryGetValue(entityHandle, out var list))
                {
                    list.Remove(behaviour);

                    // 如果列表为空，移除 Entity 映射
                    if (list.Count == 0)
                    {
                        _entityToBehaviours.Remove(entityHandle);
                    }
                }
            }

            _allBehaviours.Remove(behaviour);
            count++;
        }

        return count;
    }

    // ========================================================================
    // 清理方法
    // ========================================================================

    /// <summary>
    /// 清除所有注册。
    /// </summary>
    /// <remarks>
    /// ⚠️ 会调用所有 Behaviour 的 OnDestroy。
    /// </remarks>
    public void Clear()
    {
        // 调用所有 Behaviour 的 OnDestroy
        foreach (var behaviour in _allBehaviours)
        {
            if (!behaviour.IsDestroyed)
            {
                behaviour.OnDestroy();
            }
        }

        // 清空所有映射
        _entityToBehaviours.Clear();
        _behaviourToEntity.Clear();
        _allBehaviours.Clear();
        _pendingDestroy.Clear();
    }
}
