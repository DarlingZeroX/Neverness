using Neverness.Runtime.Scene;

// ============================================================================
// ScriptBehaviourScheduler.cs - 脚本行为调度器
// ============================================================================
// 管理所有 EntityBehaviour 实例的生命周期。
// 作为 ISceneSystem 注册到 SceneSystemScheduler。
//
// 性能优化：
// - 使用 BehaviourRegistry 管理 Entity ↔ Behaviour 映射
// - 使用 Queue 处理待创建/待启动/待销毁队列
// - 避免在遍历过程中修改集合
// ============================================================================

using Neverness.Runtime.Scene;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本行为调度器：管理所有 EntityBehaviour 实例的生命周期。
/// </summary>
/// <remarks>
/// ⚠️ Behaviour 的 ownership 归 Scheduler，不归 Entity。
///
/// 生命周期顺序：
/// 1. ProcessPendingCreate() - 本帧创建的 Behaviour，调用 OnCreate
/// 2. ProcessPendingStart()  - 上一帧创建的 Behaviour，调用 OnStart（延迟 1 帧）
/// 3. 遍历 _activeBehaviours，调用 OnUpdate
/// 4. ProcessPendingDestroy() - 销毁标记的 Behaviour，调用 OnDestroy
///
/// 性能特点：
/// - O(1) 注册和销毁
/// - O(n) 遍历（n 为活跃 Behaviour 数量）
/// - 使用 IReadOnlyList 避免 GC 分配
/// </remarks>
[SceneSystemTag(SceneSystemTags.Gameplay)]
public sealed class ScriptBehaviourScheduler : ISceneSystem, ISystemInitialize, ISystemTick, ISystemFixedTick, ISystemLateTick, ISystemShutdown
{
    // ========================================================================
    // 单例
    // ========================================================================

    /// <summary>全局实例（用于 Entity 访问）。</summary>
    public static ScriptBehaviourScheduler? Instance { get; private set; }

    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>Behaviour 注册表。</summary>
    private readonly BehaviourRegistry _registry = new();

    /// <summary>Behaviour 注册表（供 Bridge 直接查询，不经过 Scheduler）。</summary>
    public BehaviourRegistry Registry => _registry;

    /// <summary>待创建队列（本帧 OnCreate）。</summary>
    private readonly Queue<EntityBehaviour> _pendingCreate = new();

    /// <summary>待启动队列（下一帧 OnStart）。</summary>
    private readonly Queue<EntityBehaviour> _pendingStart = new();

    /// <summary>是否已初始化。</summary>
    private bool _isInitialized;

    // ========================================================================
    // ISceneSystem
    // ========================================================================

    /// <inheritdoc/>
    public string Name => "ScriptBehaviourScheduler";

    /// <inheritdoc/>
    public TickGroup TickGroup => TickGroup.Update;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建脚本行为调度器。</summary>
    public ScriptBehaviourScheduler()
    {
        Instance = this;
    }

    // ========================================================================
    // ISystemInitialize
    // ========================================================================

    /// <inheritdoc/>
    public void Initialize(SceneWorld world)
    {
        if (_isInitialized)
        {
            return;
        }

        _isInitialized = true;
    }

    // ========================================================================
    // ISystemTick
    // ========================================================================

    /// <inheritdoc/>
    public void Tick(SceneWorld world, float deltaTime)
    {
        // 1. 处理待启动的脚本（上一帧创建的）
        ProcessPendingStart();

        // 2. 处理待创建的脚本（本帧创建的）
        ProcessPendingCreate();

        // 3. 驱动 OnUpdate
        foreach (var behaviour in _registry.AllBehaviours)
        {
            if (behaviour.Enabled && !behaviour.IsDestroyed && behaviour.IsStarted)
            {
                behaviour.OnUpdate(deltaTime);
            }
        }

        // 4. 处理待销毁的脚本
        _registry.ProcessPendingDestroy();
    }

    // ========================================================================
    // ISystemFixedTick
    // ========================================================================

    /// <inheritdoc/>
    public void FixedTick(SceneWorld world, float fixedDeltaTime)
    {
        foreach (var behaviour in _registry.AllBehaviours)
        {
            if (behaviour.Enabled && !behaviour.IsDestroyed && behaviour.IsStarted)
            {
                behaviour.OnFixedUpdate(fixedDeltaTime);
            }
        }
    }

    // ========================================================================
    // ISystemLateTick
    // ========================================================================

    /// <inheritdoc/>
    public void LateTick(SceneWorld world, float deltaTime)
    {
        foreach (var behaviour in _registry.AllBehaviours)
        {
            if (behaviour.Enabled && !behaviour.IsDestroyed && behaviour.IsStarted)
            {
                behaviour.OnLateUpdate(deltaTime);
            }
        }
    }

    // ========================================================================
    // ISystemShutdown
    // ========================================================================

    /// <inheritdoc/>
    public void Shutdown(SceneWorld world)
    {
        // 清空待处理队列
        _pendingCreate.Clear();
        _pendingStart.Clear();

        // 销毁所有 Behaviour
        _registry.Clear();

        _isInitialized = false;
        Instance = null;
    }

    // ========================================================================
    // 公共 API
    // ========================================================================

    /// <summary>
    /// 注册新的 Behaviour 实例。
    /// </summary>
    /// <param name="entity">所属 Entity。</param>
    /// <param name="behaviour">Behaviour 实例。</param>
    public void Register(Entity entity, EntityBehaviour behaviour)
    {
        ArgumentNullException.ThrowIfNull(entity);
        ArgumentNullException.ThrowIfNull(behaviour);

        // 注册到映射表
        _registry.Register(entity.Id, behaviour);

        // 加入待创建队列
        _pendingCreate.Enqueue(behaviour);
    }

    /// <summary>
    /// 获取 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>Behaviour 列表。</returns>
    public IReadOnlyList<EntityBehaviour> GetBehaviours(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        return _registry.GetBehaviours(entity);
    }

    /// <summary>
    /// 标记 Behaviour 待销毁。
    /// </summary>
    /// <param name="behaviour">Behaviour 实例。</param>
    public void MarkForDestroy(EntityBehaviour behaviour)
    {
        _registry.MarkForDestroy(behaviour);
    }

    /// <summary>
    /// 销毁 Entity 的所有 Behaviour。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    public void DestroyAllBehaviours(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        _registry.DestroyAllBehaviours(entity);
    }

    /// <summary>
    /// 立即销毁所有 Behaviour（同步执行 OnDestroy）。
    /// 用于热重载：在旧 ALC 中执行完所有 OnDestroy 后再卸载。
    /// ⚠️ 不要在此帧的 Tick 中调用，会破坏生命周期顺序。
    /// </summary>
    public void DestroyAllImmediate()
    {
        // 标记所有 Behaviour 待销毁
        foreach (var behaviour in _registry.AllBehaviours.ToList())
        {
            _registry.MarkForDestroy(behaviour);
        }

        // 立即执行 OnDestroy + 清理映射
        _registry.ProcessPendingDestroy();
    }

    /// <summary>
    /// 获取所有活跃的 Behaviour 数量。
    /// </summary>
    public int ActiveBehaviourCount => _registry.Count;

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>
    /// 处理待创建队列（本帧 OnCreate）。
    /// </summary>
    private void ProcessPendingCreate()
    {
        while (_pendingCreate.Count > 0)
        {
            var behaviour = _pendingCreate.Dequeue();

            // 检查是否已销毁
            if (behaviour.IsDestroyed)
            {
                continue;
            }

            // 调用 OnCreate
            behaviour.OnCreate();

            // 加入待启动队列（延迟到下一帧 OnStart）
            _pendingStart.Enqueue(behaviour);
        }
    }

    /// <summary>
    /// 处理待启动队列（上一帧创建的，本帧 OnStart）。
    /// </summary>
    private void ProcessPendingStart()
    {
        while (_pendingStart.Count > 0)
        {
            var behaviour = _pendingStart.Dequeue();

            // 检查是否已销毁
            if (behaviour.IsDestroyed)
            {
                continue;
            }

            // 调用 OnStart
            behaviour.OnStart();
            behaviour.IsStarted = true;
        }
    }
}
