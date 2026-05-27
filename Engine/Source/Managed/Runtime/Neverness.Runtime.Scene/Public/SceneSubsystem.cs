using Neverness.Runtime.RuntimeLoop;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景子系统——将 <see cref="SceneManager.TickActiveScene"/> 接入 <see cref="RuntimeLoop"/> 调度。
/// 在 Update 阶段驱动激活场景的 ECS System Tick。
/// </summary>
public sealed class SceneSubsystem : IManagedRuntimeSubsystem
{
    private readonly SceneManager _sceneManager;

    public SceneSubsystem(SceneManager sceneManager)
    {
        ArgumentNullException.ThrowIfNull(sceneManager);
        _sceneManager = sceneManager;
    }

    /// <summary>在 Update 阶段 Tick，与 Native SceneTick 对齐。</summary>
    public RuntimeTickGroup TickGroup => RuntimeTickGroup.Update;

    public void Initialize()
    {
        // 无需额外初始化；SceneManager 由 Editor 生命周期管理。
    }

    public void Tick(in ManagedRuntimeFrameContext context)
    {
        _sceneManager.TickActiveScene(context.DeltaTimeSeconds);
    }

    public void Shutdown()
    {
        // 无需额外清理；SceneManager 由 Editor 生命周期管理。
    }
}
