using Neverness.Runtime.RuntimeLoop;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景子系统——将 SceneManager.TickActiveScene 接入 RuntimeLoop 调度。
/// 在 Update 阶段驱动激活场景的 ECS System Tick。
///
/// 支持 Editor 侧通过 TickOverride 注入自定义 tick 逻辑（如 PlayModeController 驱动的标签过滤）。
/// </summary>
public sealed class SceneSubsystem : IManagedRuntimeSubsystem
{
    private readonly SceneManager _sceneManager;

    /// <summary>
    /// Tick 委托覆盖——Editor 侧设置后，替换默认的 SceneManager.TickActiveScene 调用。
    /// 委托返回 true 表示已处理，不再调用默认逻辑；返回 false 回退到默认行为。
    /// </summary>
    public static Func<float, bool>? TickOverride { get; set; }

    public SceneSubsystem(SceneManager sceneManager)
    {
        ArgumentNullException.ThrowIfNull(sceneManager);
        _sceneManager = sceneManager;
    }

    /// <summary>在 Update 阶段 Tick。</summary>
    public RuntimeTickGroup TickGroup => RuntimeTickGroup.Update;

    public void Initialize()
    {
        // 无需额外初始化；SceneManager 由 Editor 生命周期管理。
    }

    public void Tick(in ManagedRuntimeFrameContext context)
    {
        // 优先使用 Editor 注入的委托（PlayModeController 驱动标签过滤 tick）
        if (TickOverride != null && TickOverride(context.DeltaTimeSeconds))
        {
            return;
        }

        // 默认行为：无条件 tick 全部系统
        _sceneManager.TickActiveScene(context.DeltaTimeSeconds);
    }

    public void Shutdown()
    {
        // 无需额外清理；SceneManager 由 Editor 生命周期管理。
    }
}
