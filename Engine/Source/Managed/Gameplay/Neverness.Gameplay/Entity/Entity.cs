// ============================================================================
// Entity.cs - 实体高级门面
// ============================================================================
// 封装 SceneEntity，提供面向游戏开发者的 API。
// Entity 不拥有 Behaviour 实例，Behaviour 由 ScriptBehaviourScheduler 持有。
// ============================================================================

using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Gameplay;

/// <summary>
/// 实体高级门面：封装 SceneEntity，提供面向游戏开发者的 API。
/// </summary>
/// <remarks>
/// ⚠️ Entity 不拥有 Behaviour 实例，Behaviour 由 ScriptBehaviourScheduler 持有。
/// ⚠️ Component API 是 Native ECS 的 proxy view，不允许脱离 SceneWorld 单独存在。
/// </remarks>
public sealed class Entity
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>底层 SceneEntity（内部使用）。</summary>
    internal SceneEntity SceneEntity { get; }

    /// <summary>所属场景世界（内部使用）。</summary>
    internal SceneWorld SceneWorld { get; }

    // ========================================================================
    // 公共属性
    // ========================================================================

    /// <summary>实体 ID（唯一标识）。</summary>
    public int Id => SceneEntity.Id;

    /// <summary>实体名称。</summary>
    public string Name
    {
        get => SceneEntity.DisplayName;
        set => SceneEntity.DisplayName = value;
    }

    /// <summary>实体是否存活。</summary>
    public bool IsAlive => SceneEntity.IsAlive;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>由 SceneEntity 和 SceneWorld 建立门面。</summary>
    public Entity(SceneEntity sceneEntity, SceneWorld sceneWorld)
    {
        SceneEntity = sceneEntity ?? throw new ArgumentNullException(nameof(sceneEntity));
        SceneWorld = sceneWorld ?? throw new ArgumentNullException(nameof(sceneWorld));
    }

    // ========================================================================
    // 组件操作（ECS Proxy View）
    // ========================================================================

    /// <summary>
    /// 添加组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>新添加的组件值。</returns>
    /// <remarks>
    /// ⚠️ 返回的是 ECS 组件的 proxy view，修改会直接反映到 ECS。
    /// 组件的生命周期由 ECS 管理，Entity 销毁时组件自动移除。
    /// </remarks>
    public T AddComponent<T>() where T : struct, IComponent
    {
        var data = new T();
        SceneEntity.AddComponent(data);
        return data;
    }

    /// <summary>
    /// 添加组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="data">组件数据。</param>
    public void AddComponent<T>(T data) where T : struct, IComponent
    {
        SceneEntity.AddComponent(data);
    }

    /// <summary>
    /// 获取组件的引用。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>组件引用。</returns>
    public ref T GetComponent<T>() where T : struct, IComponent
    {
        return ref SceneEntity.GetComponent<T>();
    }

    /// <summary>
    /// 尝试获取组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="component">输出的组件值。</param>
    /// <returns>是否成功获取。</returns>
    public bool TryGetComponent<T>(out T component) where T : struct, IComponent
    {
        return SceneEntity.TryGetComponent(out component);
    }

    /// <summary>
    /// 写入组件数据。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <param name="data">要写入的组件数据。</param>
    /// <remarks>
    /// ⚠️ 直接覆盖 ECS 数据，立即生效。
    /// </remarks>
    public void SetComponent<T>(T data) where T : struct, IComponent
    {
        SceneEntity.SetComponent(data);
    }

    /// <summary>
    /// 移除组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    public void RemoveComponent<T>() where T : struct, IComponent
    {
        SceneEntity.RemoveComponent<T>();
    }

    /// <summary>
    /// 检查是否拥有组件。
    /// </summary>
    /// <typeparam name="T">组件类型（必须是 struct，实现 IComponent）。</typeparam>
    /// <returns>是否拥有该组件。</returns>
    public bool HasComponent<T>() where T : struct, IComponent
    {
        return SceneEntity.HasComponent<T>();
    }

    // ========================================================================
    // 层级操作
    // ========================================================================

    /// <summary>设置父实体。</summary>
    /// <param name="parent">父实体。</param>
    public void SetParent(Entity parent)
    {
        ArgumentNullException.ThrowIfNull(parent);
        SceneEntity.SetParent(parent.SceneEntity);
    }

    /// <summary>获取父实体 ID；无父时返回 -1。</summary>
    /// <returns>父实体 ID。</returns>
    internal int GetParentId()
    {
        var parent = SceneEntity.GetParent();
        return parent?.Id ?? -1;
    }

    // ========================================================================
    // 脚本操作
    // ========================================================================

    /// <summary>
    /// 添加脚本组件。
    /// </summary>
    /// <typeparam name="T">脚本类型（必须继承 EntityBehaviour）。</typeparam>
    /// <returns>新创建的 Behaviour 实例。</returns>
    /// <remarks>
    /// ⚠️ 创建 Behaviour 实例并注册到 ScriptBehaviourScheduler。
    /// Entity 不持有 Behaviour 的 ownership。
    /// </remarks>
    public T AddBehaviour<T>() where T : EntityBehaviour, new()
    {
        var behaviour = new T();
        behaviour.Entity = this;

        // 注册到 ScriptBehaviourScheduler
        ScriptBehaviourScheduler.Instance?.Register(this, behaviour);

        return behaviour;
    }

    /// <summary>
    /// 获取指定类型的脚本组件。
    /// </summary>
    /// <typeparam name="T">脚本类型。</typeparam>
    /// <returns>Behaviour 实例，未找到时返回 null。</returns>
    public T? GetBehaviour<T>() where T : EntityBehaviour
    {
        return ScriptBehaviourScheduler.Instance?.GetBehaviours(this)
            .OfType<T>()
            .FirstOrDefault();
    }

    /// <summary>
    /// 获取所有脚本组件。
    /// </summary>
    /// <returns>Behaviour 列表。</returns>
    public IReadOnlyList<EntityBehaviour> GetBehaviours()
    {
        return ScriptBehaviourScheduler.Instance?.GetBehaviours(this)
            ?? Array.Empty<EntityBehaviour>();
    }

    // ========================================================================
    // 生命周期
    // ========================================================================

    /// <summary>
    /// 销毁实体。
    /// </summary>
    /// <remarks>
    /// 销毁会触发所有关联 Behaviour 的 OnDestroy 回调。
    /// </remarks>
    public void Destroy()
    {
        // 销毁所有关联的 Behaviour
        ScriptBehaviourScheduler.Instance?.DestroyAllBehaviours(this);

        // 销毁 Native Entity
        SceneWorld.DestroyEntity(SceneEntity);
    }

    // ========================================================================
    // Object 覆写
    // ========================================================================

    /// <inheritdoc/>
    public override string ToString() => $"Entity({Name}, Id={Id:X})";

    /// <inheritdoc/>
    public override bool Equals(object? obj)
    {
        return obj is Entity other && Id == other.Id;
    }

    /// <inheritdoc/>
    public override int GetHashCode() => Id.GetHashCode();
}
