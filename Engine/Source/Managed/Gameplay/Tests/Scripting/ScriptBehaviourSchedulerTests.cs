// ============================================================================
// ScriptBehaviourSchedulerTests.cs - ScriptBehaviourScheduler 单元测试
// ============================================================================

using Neverness.Gameplay;
using Xunit;

namespace Neverness.Gameplay.Tests.Scripting;

/// <summary>
/// ScriptBehaviourScheduler 单元测试。
/// </summary>
public class ScriptBehaviourSchedulerTests
{
    // ========================================================================
    // 测试用脚本类
    // ========================================================================

    private class TestBehaviour : EntityBehaviour
    {
        public int OnCreateCallCount { get; private set; }
        public int OnStartCallCount { get; private set; }
        public int OnUpdateCallCount { get; private set; }
        public int OnDestroyCallCount { get; private set; }
        public float LastDeltaTime { get; private set; }

        public override void OnCreate()
        {
            OnCreateCallCount++;
        }

        public override void OnStart()
        {
            OnStartCallCount++;
        }

        public override void OnUpdate(float deltaTime)
        {
            OnUpdateCallCount++;
            LastDeltaTime = deltaTime;
        }

        public override void OnDestroy()
        {
            OnDestroyCallCount++;
        }
    }

    // ========================================================================
    // 生命周期测试
    // ========================================================================

    [Fact]
    public void Register_QueuesBehaviourForCreation()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // OnCreate 尚未调用（等待 Tick）
        Assert.Equal(0, behaviour.OnCreateCallCount);
    }

    [Fact]
    public void Tick_CallsOnCreateOnFirstTick()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：调用 OnCreate
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(1, behaviour.OnCreateCallCount);
        Assert.Equal(0, behaviour.OnStartCallCount);  // OnStart 延迟 1 帧
    }

    [Fact]
    public void Tick_CallsOnStartOnSecondTick()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：调用 OnCreate
        scheduler.Tick(null!, 0.016f);

        // 第二次 Tick：调用 OnStart
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(1, behaviour.OnCreateCallCount);
        Assert.Equal(1, behaviour.OnStartCallCount);
    }

    [Fact]
    public void Tick_CallsOnUpdateAfterOnStart()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：OnCreate
        scheduler.Tick(null!, 0.016f);

        // 第二次 Tick：OnStart + OnUpdate
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(1, behaviour.OnUpdateCallCount);
        Assert.Equal(0.016f, behaviour.LastDeltaTime, 4);
    }

    [Fact]
    public void Tick_CallsOnUpdateEveryFrame()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：OnCreate
        scheduler.Tick(null!, 0.016f);

        // 第二次 Tick：OnStart + OnUpdate
        scheduler.Tick(null!, 0.016f);

        // 第三次 Tick：OnUpdate
        scheduler.Tick(null!, 0.016f);

        // 第四次 Tick：OnUpdate
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(3, behaviour.OnUpdateCallCount);
    }

    [Fact]
    public void MarkForDestroy_CallsOnDestroyOnNextTick()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：OnCreate
        scheduler.Tick(null!, 0.016f);

        // 第二次 Tick：OnStart + OnUpdate
        scheduler.Tick(null!, 0.016f);

        // 标记销毁
        scheduler.MarkForDestroy(behaviour);

        // 第三次 Tick：OnDestroy
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(1, behaviour.OnDestroyCallCount);
    }

    [Fact]
    public void DisabledBehaviour_SkipsOnUpdate()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour);

        // 第一次 Tick：OnCreate
        scheduler.Tick(null!, 0.016f);

        // 第二次 Tick：OnStart
        scheduler.Tick(null!, 0.016f);

        // 禁用
        behaviour.Enabled = false;

        // 第三次 Tick：OnUpdate 应该被跳过
        scheduler.Tick(null!, 0.016f);

        Assert.Equal(0, behaviour.OnUpdateCallCount);
    }

    [Fact]
    public void ActiveBehaviourCount_ReturnsCorrectCount()
    {
        var scheduler = new ScriptBehaviourScheduler();
        var behaviour1 = new TestBehaviour();
        var behaviour2 = new TestBehaviour();
        var entity = new Entity(new Neverness.Runtime.Scene.SceneEntity(new Neverness.Runtime.Scene.NNEntityHandle(1, 0)), null!);

        scheduler.Register(entity, behaviour1);
        scheduler.Register(entity, behaviour2);

        Assert.Equal(2, scheduler.ActiveBehaviourCount);
    }
}
