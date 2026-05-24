namespace Neverness.Runtime.Scene;

/// <summary>每帧 Tick 系统——由 Scheduler 在指定 <see cref="TickGroup"/> 调用。</summary>
public interface ISystemTick : ISceneSystem
{
    /// <summary>Tick 分组，默认 Update。</summary>
    TickGroup TickGroup => TickGroup.Update;

    void Tick(SceneWorld world, float deltaTime);
}
