// ============================================================================
// ScriptBehaviourBridge.cs - 脚本行为桥接层
// ============================================================================
// 桥接 ECS ScriptComponent 与 Behaviour 实例。
//
// 职责：
// - 发现有 ScriptComponent 但无 Behaviour → 创建
// - 发现 ScriptTypeId 变化 → 销毁旧 + 创建新
// - 发现 ScriptComponent 移除 → 销毁
// - 同步 Enabled 状态
//
// 不依赖 Scheduler 查询，只操作 BehaviourRegistry。
//
// Phase 1: 每帧全量扫描 SceneQuery<NNScriptComponentData>
// Phase 2: 改为 entt on_construct / on_update / on_destroy 事件驱动
// ============================================================================

using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本行为桥接层——桥接 ECS ScriptComponent 与 Behaviour 实例。
/// 实现 ISystemTick 注册到 SceneSystemScheduler，每帧自动同步。
/// </summary>
[SceneSystemTag(SceneSystemTags.Gameplay)]
public sealed class ScriptBehaviourBridge : ISceneSystem, ISystemTick
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    private readonly IScriptFactory _factory;
    private readonly BehaviourRegistry _registry;
    private readonly ScriptRegistry _scriptRegistry;
    private readonly ScriptBehaviourScheduler _scheduler;

    /// <summary>上一帧有 ScriptComponent 的 Entity 句柄集合（用于检测 Component 删除）。</summary>
    private readonly HashSet<ulong> _previousScriptEntities = new();

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建脚本行为桥接层。</summary>
    public ScriptBehaviourBridge(
        IScriptFactory factory,
        BehaviourRegistry registry,
        ScriptRegistry scriptRegistry,
        ScriptBehaviourScheduler scheduler)
    {
        _factory = factory;
        _registry = registry;
        _scriptRegistry = scriptRegistry;
        _scheduler = scheduler;
    }

    // ========================================================================
    // ISceneSystem
    // ========================================================================

    /// <inheritdoc/>
    public string Name => "ScriptBehaviourBridge";

    // ========================================================================
    // ISystemTick
    // ========================================================================

    /// <inheritdoc/>
    public TickGroup TickGroup => TickGroup.EarlyUpdate;

    /// <inheritdoc/>
    public void Tick(SceneWorld world, float deltaTime)
    {
        Sync(world);
    }

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 每帧调用，同步 ECS ScriptComponent → Behaviour。
    /// </summary>
    /// <param name="world">ECS 场景世界。</param>
    public void Sync(SceneWorld world)
    {
        var currentEntities = new HashSet<ulong>();

        // 查询所有有 ScriptComponent 的 Entity
        var query = world.GetQuery<NNScriptComponentData>();
        var view = query.Execute();

        for (int i = 0; i < view.Count; i++)
        {
            ulong entityHandle = view.GetEntity(i).Value;
            ref var script = ref view.GetComponent(i);
            currentEntities.Add(entityHandle);

            // 创建 SceneEntity 和 Entity 门面
            var entityHandle2 = view.GetEntity(i);
            var sceneEntity = new SceneEntity(entityHandle2, world.NativeHandle);
            var entity = new Entity(sceneEntity, world);

            // 直接查 BehaviourRegistry，不经过 Scheduler
            _registry.TryGet(entity, out var existing);

            if (script.ScriptTypeId == 0)
            {
                // 无脚本 → 销毁 Behaviour
                if (existing != null)
                    _scheduler.MarkForDestroy(existing);
                continue;
            }

            var scriptInfo = _scriptRegistry.FindByTypeId(script.ScriptTypeId);
            if (scriptInfo == null)
                continue; // 未编译 / 未注册

            if (existing == null)
            {
                // 无 Behaviour → 创建
                var behaviour = _factory.Create(scriptInfo.Type);
                if (behaviour != null)
                {
                    behaviour.Entity = entity;
                    behaviour.Enabled = script.Enabled != 0;
                    _scheduler.Register(entity, behaviour);
                }
            }
            else if (existing.GetType() != scriptInfo.Type)
            {
                // 类型变化 → 销毁旧 + 创建新
                _scheduler.MarkForDestroy(existing);
                var behaviour = _factory.Create(scriptInfo.Type);
                if (behaviour != null)
                {
                    behaviour.Entity = entity;
                    behaviour.Enabled = script.Enabled != 0;
                    _scheduler.Register(entity, behaviour);
                }
            }
            else
            {
                // 同步 Enabled
                existing.Enabled = script.Enabled != 0;
            }
        }

        // 检测 ScriptComponent 被删除的 Entity（上一帧有，这一帧没有）
        foreach (var entityId in _previousScriptEntities)
        {
            if (!currentEntities.Contains(entityId))
            {
                // ScriptComponent 被移除 → 销毁 Behaviour
                var sceneEntity = new SceneEntity(new NNEntityHandle(entityId), world.NativeHandle);
                var entity = new Entity(sceneEntity, world);
                if (_registry.TryGet(entity, out var existing) && existing != null)
                {
                    _scheduler.MarkForDestroy(existing);
                }
            }
        }

        // 更新上一帧集合
        _previousScriptEntities.Clear();
        foreach (var id in currentEntities)
            _previousScriptEntities.Add(id);
    }
}
