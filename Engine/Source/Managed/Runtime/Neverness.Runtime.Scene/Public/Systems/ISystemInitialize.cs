namespace Neverness.Runtime.Scene;

/// <summary>可初始化 System——在 System 注册后由 Scheduler 调用一次。</summary>
public interface ISystemInitialize : ISceneSystem
{
    void Initialize(SceneWorld world);
}
