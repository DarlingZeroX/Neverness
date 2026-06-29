// ============================================================================
// EntityBehaviour.cs - 实体行为脚本基类
// ============================================================================
// 用户脚本继承此类，实现游戏逻辑。
// EntityBehaviour 是 ScriptComponent 的 runtime projection，
// 由 ScriptBehaviourScheduler 唯一持有生命周期。
// ============================================================================

using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;

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
    /// 变换组件（快捷访问）。
    /// </summary>
    public ref TransformComponent Transform
    {
        get
        {
            return ref Entity.GetComponent<TransformComponent>();
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
    /// 获取组件引用——直接修改生效，无需 SetComponent。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>组件的 ref 引用，修改直接反映到 ECS。</returns>
    /// <remarks>
    /// ⚠️ 组件不存在时抛异常。对核心组件可在 OnCreate 中验证，对可选组件请先使用 HasComponent 检查。
    ///
    /// <code>
    /// // ✅ 正确：ref 直接修改
    /// ref var transform = ref GetComponent&lt;TransformComponent&gt;();
    /// transform.Position.X += Speed * deltaTime;
    ///
    /// // ❌ 错误：值拷贝，修改不会生效
    /// TransformComponent transform = GetComponent&lt;TransformComponent&gt;();
    /// transform.Position.X = 10;  // 修改的是拷贝，不影响 ECS
    /// </code>
    /// </remarks>
    protected ref T GetComponent<T>() where T : struct, IComponent
    {
        return ref Entity.GetComponent<T>();
    }

    /// <summary>
    /// 尝试获取组件（只读快照，适用于条件判断和数据读取）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="component">输出的组件值拷贝。</param>
    /// <returns>是否成功获取。</returns>
    /// <remarks>
    /// ⚠️ out 参数是值拷贝，修改不会反映到 ECS。
    /// 如需修改组件，请使用 HasComponent + GetComponent 组合：
    /// <code>
    /// if (HasComponent&lt;TransformComponent&gt;())
    /// {
    ///     ref var transform = ref GetComponent&lt;TransformComponent&gt;();
    ///     transform.Position.X = 10;  // 直接生效
    /// }
    /// </code>
    /// </remarks>
    protected bool TryGetComponent<T>(out T component) where T : struct, IComponent
    {
        return Entity.TryGetComponent(out component);
    }

    /// <summary>
    /// 写入组件数据（批量覆盖整个组件）。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="data">要写入的组件数据。</param>
    /// <remarks>
    /// 适用于从外部数据源恢复组件状态。日常单字段修改请直接使用 ref GetComponent：
    /// <code>
    /// // ✅ 推荐：ref 直接修改单个字段
    /// ref var transform = ref GetComponent&lt;TransformComponent&gt;();
    /// transform.Position.X = 10;
    ///
    /// // 适用场景：从序列化数据恢复整个组件
    /// SetComponent(savedTransformData);
    /// </code>
    /// </remarks>
    protected void SetComponent<T>(T data) where T : struct, IComponent
    {
        Entity.SetComponent(data);
    }

    /// <summary>
    /// 添加组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>新添加的组件值。</returns>
    protected T AddComponent<T>() where T : struct, IComponent
    {
        return Entity.AddComponent<T>();
    }

    /// <summary>
    /// 移除组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    protected void RemoveComponent<T>() where T : struct, IComponent
    {
        Entity.RemoveComponent<T>();
    }

    /// <summary>
    /// 检查是否拥有组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>是否拥有该组件。</returns>
    protected bool HasComponent<T>() where T : struct, IComponent
    {
        return Entity.HasComponent<T>();
    }

    // ========================================================================
    // Instantiate 方法（类似 Unity）
    // ========================================================================

    /// <summary>
    /// 克隆实体（类似 Unity 的 Instantiate）。
    /// </summary>
    /// <param name="source">源实体。</param>
    /// <returns>克隆的新实体。</returns>
    /// <remarks>
    /// ⚠️ 仅克隆组件数据，不克隆 Behaviour 脚本。
    /// 如需为克隆体添加 Behaviour，请手动调用 AddBehaviour。
    /// </remarks>
    protected Entity Instantiate(Entity source)
    {
        return source.Clone();
    }

    /// <summary>
    /// 克隆实体并设置世界位置和旋转（类似 Unity 的 Instantiate）。
    /// </summary>
    /// <param name="source">源实体。</param>
    /// <param name="position">世界位置（System.Numerics.Vector3）。</param>
    /// <param name="rotation">世界旋转（System.Numerics.Quaternion）。</param>
    /// <returns>克隆的新实体。</returns>
    protected Entity Instantiate(Entity source, System.Numerics.Vector3 position, System.Numerics.Quaternion? rotation = null)
    {
        var clone = source.Clone();

        ref var transform = ref clone.GetComponent<TransformComponent>();
        transform.Position = position;
        transform.Rotation = rotation ?? System.Numerics.Quaternion.Identity;

        return clone;
    }

    /// <summary>
    /// 克隆实体并设置为指定父实体的子实体（类似 Unity 的 Instantiate）。
    /// </summary>
    /// <param name="source">源实体。</param>
    /// <param name="parent">父实体。</param>
    /// <param name="worldPositionStays">是否保持世界位置（默认 true，与 Unity 行为一致）。</param>
    /// <returns>克隆的新实体。</returns>
    protected Entity Instantiate(Entity source, Entity parent, bool worldPositionStays = true)
    {
        var clone = source.Clone();

        if (worldPositionStays)
        {
            // 保持世界位置：计算相对父实体的局部位置
            if (parent.HasComponent<TransformComponent>())
            {
                ref var parentTransform = ref parent.GetComponent<TransformComponent>();
                ref var cloneTransform = ref clone.GetComponent<TransformComponent>();

                // 简化处理：直接使用世界位置（忽略旋转和缩放的影响）
                // TODO: 完整的逆变换计算
                cloneTransform.Position = cloneTransform.Position - parentTransform.Position;
            }
        }

        clone.SetParent(parent);
        return clone;
    }
}
