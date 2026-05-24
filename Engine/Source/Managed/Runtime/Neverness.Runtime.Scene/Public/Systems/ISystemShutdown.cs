namespace Neverness.Runtime.Scene;

/// <summary>可关闭 System——在 Scheduler Dispose 时调用。</summary>
public interface ISystemShutdown : ISceneSystem
{
    void Shutdown(SceneWorld world);
}
