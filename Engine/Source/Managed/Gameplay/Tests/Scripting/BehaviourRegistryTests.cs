// ============================================================================
// BehaviourRegistryTests.cs - BehaviourRegistry 单元测试
// ============================================================================

using Neverness.Gameplay;
using Xunit;

namespace Neverness.Gameplay.Tests.Scripting;

/// <summary>
/// BehaviourRegistry 单元测试。
/// </summary>
public class BehaviourRegistryTests
{
    // ========================================================================
    // 测试用脚本类
    // ========================================================================

    private class TestBehaviour : EntityBehaviour
    {
        public bool OnDestroyCalled { get; private set; }

        public override void OnDestroy()
        {
            OnDestroyCalled = true;
        }
    }

    // ========================================================================
    // 注册测试
    // ========================================================================

    [Fact]
    public void Register_AddsBehaviour()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);

        Assert.Equal(1, registry.Count);
    }

    [Fact]
    public void Register_SameBehaviourTwice_DoesNotDuplicate()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);
        registry.Register(1, behaviour);

        Assert.Equal(1, registry.Count);
    }

    [Fact]
    public void Register_MultipleBehavioursForSameEntity()
    {
        var registry = new BehaviourRegistry();
        var behaviour1 = new TestBehaviour();
        var behaviour2 = new TestBehaviour();

        registry.Register(1, behaviour1);
        registry.Register(1, behaviour2);

        Assert.Equal(2, registry.Count);
    }

    // ========================================================================
    // 查询测试
    // ========================================================================

    [Fact]
    public void GetBehaviours_ReturnsCorrectBehaviours()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);

        var behaviours = registry.GetBehaviours(1);

        Assert.Single(behaviours);
        Assert.Same(behaviour, behaviours[0]);
    }

    [Fact]
    public void GetBehaviours_UnknownEntity_ReturnsEmpty()
    {
        var registry = new BehaviourRegistry();

        var behaviours = registry.GetBehaviours(999);

        Assert.Empty(behaviours);
    }

    [Fact]
    public void TryGetEntityHandle_ReturnsCorrectHandle()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(42, behaviour);

        var found = registry.TryGetEntityId(behaviour, out var entityId);

        Assert.True(found);
        Assert.Equal(42, entityId);
    }

    [Fact]
    public void TryGetEntityHandle_UnknownBehaviour_ReturnsFalse()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        var found = registry.TryGetEntityId(behaviour, out var entityId);

        Assert.False(found);
        Assert.Equal(0, entityId);
    }

    // ========================================================================
    // 销毁测试
    // ========================================================================

    [Fact]
    public void MarkForDestroy_MarksBehaviourAsDestroyed()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);
        registry.MarkForDestroy(behaviour);

        Assert.True(behaviour.IsDestroyed);
    }

    [Fact]
    public void ProcessPendingDestroy_CallsOnDestroy()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);
        registry.MarkForDestroy(behaviour);
        registry.ProcessPendingDestroy();

        Assert.True(behaviour.OnDestroyCalled);
    }

    [Fact]
    public void ProcessPendingDestroy_RemovesBehaviour()
    {
        var registry = new BehaviourRegistry();
        var behaviour = new TestBehaviour();

        registry.Register(1, behaviour);
        registry.MarkForDestroy(behaviour);
        registry.ProcessPendingDestroy();

        Assert.Equal(0, registry.Count);
    }

    [Fact]
    public void DestroyAllBehaviours_MarksAllForDestroy()
    {
        var registry = new BehaviourRegistry();
        var behaviour1 = new TestBehaviour();
        var behaviour2 = new TestBehaviour();

        registry.Register(1, behaviour1);
        registry.Register(1, behaviour2);
        registry.DestroyAllBehaviours(1);

        Assert.True(behaviour1.IsDestroyed);
        Assert.True(behaviour2.IsDestroyed);
    }

    // ========================================================================
    // 清理测试
    // ========================================================================

    [Fact]
    public void Clear_CallsOnDestroyOnAllBehaviours()
    {
        var registry = new BehaviourRegistry();
        var behaviour1 = new TestBehaviour();
        var behaviour2 = new TestBehaviour();

        registry.Register(1, behaviour1);
        registry.Register(2, behaviour2);
        registry.Clear();

        Assert.True(behaviour1.OnDestroyCalled);
        Assert.True(behaviour2.OnDestroyCalled);
        Assert.Equal(0, registry.Count);
    }
}
