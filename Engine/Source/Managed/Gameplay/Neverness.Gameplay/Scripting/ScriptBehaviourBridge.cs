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
using ScriptComponent = Neverness.Runtime.Scene.Components.ScriptComponent;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本行为桥接层——桥接 ECS ScriptComponent 与 Behaviour 实例。
/// 实现 ISceneSystem 注册到 SceneSystemScheduler，每帧自动同步。
/// </summary>
[SceneSystemTag(SceneSystemTags.Gameplay)]
public sealed class ScriptBehaviourBridge : ISceneSystem
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    private readonly IScriptFactory _factory;
    private readonly BehaviourRegistry _registry;
    private readonly ScriptRegistry _scriptRegistry;
    private readonly ScriptBehaviourScheduler _scheduler;
    private readonly Func<SceneWorld?> _getWorld; // 获取 SceneWorld 的回调
    private IScene? _scene;
    private bool _isInitialized;

    /// <summary>上一帧有 ScriptComponent 的 Entity ID 集合（用于检测 Component 删除）。</summary>
    private readonly HashSet<int> _previousScriptEntities = new();

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建脚本行为桥接层。</summary>
    public ScriptBehaviourBridge(
        IScriptFactory factory,
        BehaviourRegistry registry,
        ScriptRegistry scriptRegistry,
        ScriptBehaviourScheduler scheduler,
        Func<SceneWorld?> getWorld)
    {
        _factory = factory;
        _registry = registry;
        _scriptRegistry = scriptRegistry;
        _scheduler = scheduler;
        _getWorld = getWorld;
    }

    // ========================================================================
    // ISceneSystem
    // ========================================================================

    /// <inheritdoc/>
    public string Name => "ScriptBehaviourBridge";

    /// <inheritdoc/>
    public SceneSystemTags Tags => SceneSystemTags.Gameplay;

    /// <inheritdoc/>
    public bool IsInitialized => _isInitialized;

    /// <inheritdoc/>
    public void Initialize(IScene scene)
    {
        _scene = scene;
        _isInitialized = true;
    }

    /// <inheritdoc/>
    public void Update(float deltaTime)
    {
        // 通过回调获取 SceneWorld
        var world = _getWorld();
        if (world != null && world.IsValid)
        {
            Sync(world);
        }
    }

    /// <inheritdoc/>
    public void Shutdown()
    {
        _isInitialized = false;
        _scene = null;
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        Shutdown();
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
        var currentEntities = new HashSet<int>();

        // 查询所有有 ScriptComponent 的 Entity
        ((IScene)world).Query<ScriptComponent>().ForEach((ref ScriptComponent script, IEntity entity) =>
        {
            int entityId = entity.Id;
            currentEntities.Add(entityId);

            // 创建 Entity 门面
            var sceneEntity = new SceneEntity(entity, world);
            var gameEntity = new Entity(sceneEntity, world);

            // 直接查 BehaviourRegistry，不经过 Scheduler
            _registry.TryGet(gameEntity, out var existing);

            if (script.ScriptTypeId == 0)
            {
                // 无脚本 → 销毁 Behaviour
                if (existing != null)
                    _scheduler.MarkForDestroy(existing);
                return;
            }

            var scriptInfo = _scriptRegistry.FindByTypeId(script.ScriptTypeId);
            if (scriptInfo == null)
                return; // 未编译 / 未注册

            if (existing == null)
            {
                // 无 Behaviour → 创建
                var behaviour = _factory.Create(scriptInfo.Type);
                if (behaviour != null)
                {
                    behaviour.Entity = gameEntity;
                    behaviour.Enabled = true; // 创建时默认启用
                    _scheduler.Register(gameEntity, behaviour);
                }
            }
            else if (existing.GetType() != scriptInfo.Type)
            {
                // 类型变化 → 销毁旧 + 创建新
                _scheduler.MarkForDestroy(existing);
                var behaviour = _factory.Create(scriptInfo.Type);
                if (behaviour != null)
                {
                    behaviour.Entity = gameEntity;
                    behaviour.Enabled = true; // 创建时默认启用
                    _scheduler.Register(gameEntity, behaviour);
                }
            }
            // 注意: 不再从 script.IsInitialized 同步 Enabled
            // IsInitialized 是 [Transient] 字段，反序列化后永远是 false
            // 如果需要禁用脚本，应该使用独立的 IsDisabled 字段
        });

        // 检测 ScriptComponent 被删除的 Entity（上一帧有，这一帧没有）
        foreach (var entityId in _previousScriptEntities)
        {
            if (!currentEntities.Contains(entityId))
            {
                // ScriptComponent 被移除 → 销毁 Behaviour
                var entity = ((IScene)world).GetEntity(entityId);
                if (entity != null)
                {
                    var sceneEntity = new SceneEntity(entity, world);
                    var gameEntity = new Entity(sceneEntity, world);
                    if (_registry.TryGet(gameEntity, out var existing) && existing != null)
                    {
                        _scheduler.MarkForDestroy(existing);
                    }
                }
            }
        }

        // 更新上一帧集合
        _previousScriptEntities.Clear();
        foreach (var id in currentEntities)
            _previousScriptEntities.Add(id);
    }
}
