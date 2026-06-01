// ============================================================================
// EntityBehaviour.cs - 实体行为脚本基类
// ============================================================================
// 用户脚本继承此类，实现游戏逻辑。
// EntityBehaviour 是 ScriptComponent 的 runtime projection，
// 由 ScriptBehaviourScheduler 唯一持有生命周期。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 实体行为脚本基类：用户脚本继承此类。
/// </summary>
/// <remarks>
/// ⚠️ EntityBehaviour 是 ScriptComponent 的 runtime projection，
/// 由 ScriptBehaviourScheduler 唯一持有生命周期。
///
/// 生命周期顺序：
/// 1. OnCreate() - 组件创建时调用（禁止访问其他 Entity）
/// 2. OnStart()  - 首次 Update 前调用（至少延迟 1 帧，可安全访问其他 Entity）
/// 3. OnUpdate() - 每帧调用
/// 4. OnFixedUpdate() - 固定时间步调用
/// 5. OnLateUpdate() - 每帧末尾调用
/// 6. OnDestroy() - 组件销毁时调用
/// </remarks>
public abstract class EntityBehaviour
{
    // ========================================================================
    // 属性
    // ========================================================================

    /// <summary>所属实体（由 Scheduler 注入）。</summary>
    public Entity Entity { get; internal set; } = null!;

    /// <summary>
    /// 变换组件（快捷访问，返回 proxy view）。
    /// </summary>
    /// <remarks>
    /// ⚠️ 返回的是值的 copy，修改后需要调用 Entity.SetComponent 保存。
    /// </remarks>
    public TransformComponent Transform
    {
        get
        {
            var result = Entity.GetComponent<TransformComponent>();
            return result ?? default;
        }
    }

    /// <summary>脚本是否启用。</summary>
    public bool Enabled { get; set; } = true;

    /// <summary>脚本是否已销毁。</summary>
    public bool IsDestroyed { get; internal set; }

    /// <summary>脚本是否已启动（OnStart 已调用）。</summary>
    public bool IsStarted { get; internal set; }

    // ========================================================================
    // 生命周期回调
    // ========================================================================

    /// <summary>
    /// 组件创建时调用（Awake）。
    /// </summary>
    /// <remarks>
    /// ⚠️ 禁止在此访问其他 Entity 的组件或 Behaviour。
    /// 原因：OnCreate 在同一帧内调用，Entity graph 可能不完整。
    /// </remarks>
    public virtual void OnCreate() { }

    /// <summary>
    /// 首次 Update 前调用（Start）。
    /// </summary>
    /// <remarks>
    /// ⚠️ 保证在所有 OnCreate 之后，至少延迟 1 帧。
    /// 可以安全访问其他 Entity 的组件或 Behaviour。
    /// </remarks>
    public virtual void OnStart() { }

    /// <summary>
    /// 每帧调用。
    /// </summary>
    /// <param name="deltaTime">帧间隔时间（秒）。</param>
    public virtual void OnUpdate(float deltaTime) { }

    /// <summary>
    /// 固定时间步调用。
    /// </summary>
    /// <param name="fixedDeltaTime">固定时间步（秒）。</param>
    public virtual void OnFixedUpdate(float fixedDeltaTime) { }

    /// <summary>
    /// 每帧末尾调用。
    /// </summary>
    /// <param name="deltaTime">帧间隔时间（秒）。</param>
    public virtual void OnLateUpdate(float deltaTime) { }

    /// <summary>
    /// 组件销毁时调用。
    /// </summary>
    /// <remarks>
    /// ⚠️ 谨慎访问其他 Entity，其他 Behaviour 可能已销毁。
    /// </remarks>
    public virtual void OnDestroy() { }

    // ========================================================================
    // 便捷方法
    // ========================================================================

    /// <summary>
    /// 获取同实体上的其他脚本。
    /// </summary>
    /// <typeparam name="T">脚本类型。</typeparam>
    /// <returns>Behaviour 实例，未找到时返回 null。</returns>
    protected T? GetBehaviour<T>() where T : EntityBehaviour
    {
        return Entity.GetBehaviour<T>();
    }

    /// <summary>
    /// 获取组件（返回 proxy view）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <returns>组件值，无组件时返回 null。</returns>
    /// <remarks>
    /// ⚠️ 返回的是值的 copy，修改后需要调用 SetComponent 保存。
    /// </remarks>
    protected T? GetComponent<T>() where T : struct
    {
        return Entity.GetComponent<T>();
    }

    /// <summary>
    /// 写入组件数据。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <param name="data">要写入的组件数据。</param>
    protected void SetComponent<T>(T data) where T : struct
    {
        Entity.SetComponent(data);
    }

    /// <summary>
    /// 添加组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <returns>新添加的组件值。</returns>
    protected T AddComponent<T>() where T : struct, new()
    {
        return Entity.AddComponent<T>();
    }

    /// <summary>
    /// 移除组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <returns>是否成功移除。</returns>
    protected bool RemoveComponent<T>() where T : struct
    {
        return Entity.RemoveComponent<T>();
    }

    /// <summary>
    /// 检查是否拥有组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct）。</typeparam>
    /// <returns>是否拥有该组件。</returns>
    protected bool HasComponent<T>() where T : struct
    {
        return Entity.HasComponent<T>();
    }
}
