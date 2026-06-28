// ============================================================================
// GameplayContext.cs - Gameplay 上下文
// ============================================================================
// 包含所有 Gameplay 服务的容器，避免 static 全局污染。
// IInputProvider 已下沉到 Neverness.Runtime.Application。
// ============================================================================

using Neverness.Runtime.Application;

namespace Neverness.Gameplay;

/// <summary>
/// Gameplay 上下文：包含所有 Gameplay 服务的容器。
/// </summary>
/// <remarks>
/// ⚠️ 避免 static 全局污染，支持多实例和可测试性。
///
/// 使用示例：
/// <code>
/// // 引擎初始化时
/// var context = new GameplayContext();
/// context.Initialize();
///
/// // 用户脚本中
/// public class PlayerController : EntityBehaviour
/// {
///     public override void OnUpdate(float deltaTime)
///     {
///         if (Input.GetKeyDown(KeyCode.Space))
///         {
///             Debug.Log("Jump!");
///         }
///     }
/// }
/// </code>
/// </remarks>
public sealed class GameplayContext
{
    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>当前活动上下文（单例）。</summary>
    public static GameplayContext? Current { get; private set; }

    // ========================================================================
    // 实例属性
    // ========================================================================

    /// <summary>脚本注册表。</summary>
    public ScriptRegistry ScriptRegistry { get; } = new();

    /// <summary>脚本行为调度器。</summary>
    public ScriptBehaviourScheduler BehaviourScheduler { get; } = new();

    /// <summary>输入提供者。</summary>
    public IInputProvider? InputProvider { get; set; }

    /// <summary>时间提供者。</summary>
    public ITimeProvider? TimeProvider { get; set; }

    /// <summary>是否已初始化。</summary>
    public bool IsInitialized { get; private set; }

    // ========================================================================
    // 初始化方法
    // ========================================================================

    /// <summary>
    /// 初始化上下文。
    /// </summary>
    public void Initialize()
    {
        if (IsInitialized)
        {
            return;
        }

        // 注册到 static 访问器
        Current = this;

        // 设置 Input/Time 提供者
        if (InputProvider != null)
        {
            Input.SetProvider(InputProvider);
        }

        if (TimeProvider != null)
        {
            Time.SetProvider(TimeProvider);
        }

        IsInitialized = true;
    }

    /// <summary>
    /// 关闭上下文。
    /// </summary>
    public void Shutdown()
    {
        if (!IsInitialized)
        {
            return;
        }

        // 清理服务
        ScriptRegistry.Clear();

        // 清除 static 访问器
        if (Current == this)
        {
            Current = null;
        }

        IsInitialized = false;
    }
}
