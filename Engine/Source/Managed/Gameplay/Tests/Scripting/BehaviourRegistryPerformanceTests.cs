// ============================================================================
// BehaviourRegistryPerformanceTests.cs - BehaviourRegistry 性能测试
// ============================================================================

using Neverness.Gameplay;
using System.Diagnostics;
using Xunit;
using Xunit.Abstractions;

namespace Neverness.Gameplay.Tests.Scripting;

/// <summary>
/// BehaviourRegistry 性能测试。
/// </summary>
public class BehaviourRegistryPerformanceTests
{
    private readonly ITestOutputHelper _output;

    public BehaviourRegistryPerformanceTests(ITestOutputHelper output)
    {
        _output = output;
    }

    // ========================================================================
    // 测试用脚本类
    // ========================================================================

    private class TestBehaviour : EntityBehaviour { }

    // ========================================================================
    // 注册性能测试
    // ========================================================================

    [Fact]
    public void Register_1000Behaviours_CompletesInReasonableTime()
    {
        var registry = new BehaviourRegistry();
        var stopwatch = Stopwatch.StartNew();

        for (int i = 0; i < 1000; i++)
        {
            var behaviour = new TestBehaviour();
            registry.Register(i, behaviour);
        }

        stopwatch.Stop();

        _output.WriteLine($"Registered 1000 behaviours in {stopwatch.ElapsedMilliseconds}ms");

        Assert.Equal(1000, registry.Count);
        Assert.True(stopwatch.ElapsedMilliseconds < 100, "Registration should be fast");
    }

    [Fact]
    public void GetBehaviours_LookupPerformance()
    {
        var registry = new BehaviourRegistry();

        // 预先注册 1000 个 Behaviour
        for (int i = 0; i < 1000; i++)
        {
            registry.Register(i, new TestBehaviour());
        }

        var stopwatch = Stopwatch.StartNew();

        // 执行 10000 次查找
        for (int i = 0; i < 10000; i++)
        {
            registry.GetBehaviours(i % 1000);
        }

        stopwatch.Stop();

        _output.WriteLine($"10000 lookups in {stopwatch.ElapsedMilliseconds}ms");

        Assert.True(stopwatch.ElapsedMilliseconds < 500, "Lookup should be fast");
    }

    [Fact]
    public void ProcessPendingDestroy_100Behaviours_CompletesInReasonableTime()
    {
        var registry = new BehaviourRegistry();

        // 预先注册 100 个 Behaviour
        for (int i = 0; i < 100; i++)
        {
            registry.Register(i, new TestBehaviour());
        }

        // 标记所有 Behaviour 待销毁
        foreach (var behaviour in registry.AllBehaviours)
        {
            registry.MarkForDestroy(behaviour);
        }

        var stopwatch = Stopwatch.StartNew();

        // 处理销毁队列
        var count = registry.ProcessPendingDestroy();

        stopwatch.Stop();

        _output.WriteLine($"Processed {count} destroy operations in {stopwatch.ElapsedMilliseconds}ms");

        Assert.Equal(100, count);
        Assert.True(stopwatch.ElapsedMilliseconds < 100, "Destroy processing should be fast");
    }

    [Fact]
    public void Register_MultipleBehavioursForSameEntity_Performance()
    {
        var registry = new BehaviourRegistry();
        var stopwatch = Stopwatch.StartNew();

        // 同一个 Entity 注册 100 个 Behaviour
        for (int i = 0; i < 100; i++)
        {
            registry.Register(1, new TestBehaviour());
        }

        stopwatch.Stop();

        _output.WriteLine($"Registered 100 behaviours for same entity in {stopwatch.ElapsedMilliseconds}ms");

        var behaviours = registry.GetBehaviours(1);
        Assert.Equal(100, behaviours.Count);
        Assert.True(stopwatch.ElapsedMilliseconds < 50, "Registration should be fast");
    }
}
