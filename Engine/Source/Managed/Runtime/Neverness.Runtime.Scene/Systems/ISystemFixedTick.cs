namespace Neverness.Runtime.Scene;

/// <summary>固定步长 Tick 系统——由 Scheduler 在 <see cref="TickGroup.FixedUpdate"/> 调用。</summary>
public interface ISystemFixedTick : ISceneSystem
{
    TickGroup TickGroup => TickGroup.FixedUpdate;

    void FixedTick(SceneWorld world, float fixedDeltaTime);
}
