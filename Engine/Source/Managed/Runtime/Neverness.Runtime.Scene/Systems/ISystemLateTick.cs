namespace Neverness.Runtime.Scene;

/// <summary>每帧 Late Tick 系统——由 Scheduler 在 <see cref="TickGroup.LateUpdate"/> 调用。</summary>
public interface ISystemLateTick : ISceneSystem
{
    TickGroup TickGroup => TickGroup.LateUpdate;

    void LateTick(SceneWorld world, float deltaTime);
}
