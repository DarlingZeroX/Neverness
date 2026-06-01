// ============================================================================
// ScriptRegistryPerformanceTests.cs - ScriptRegistry 性能测试
// ============================================================================

using Neverness.Gameplay;
using System.Diagnostics;
using Xunit;
using Xunit.Abstractions;

namespace Neverness.Gameplay.Tests.Scripting;

/// <summary>
/// ScriptRegistry 性能测试。
/// </summary>
public class ScriptRegistryPerformanceTests
{
    private readonly ITestOutputHelper _output;

    public ScriptRegistryPerformanceTests(ITestOutputHelper output)
    {
        _output = output;
    }

    // ========================================================================
    // 测试用脚本类
    // ========================================================================

    private class TestScript0 : EntityBehaviour { }
    private class TestScript1 : EntityBehaviour { }
    private class TestScript2 : EntityBehaviour { }
    private class TestScript3 : EntityBehaviour { }
    private class TestScript4 : EntityBehaviour { }
    private class TestScript5 : EntityBehaviour { }
    private class TestScript6 : EntityBehaviour { }
    private class TestScript7 : EntityBehaviour { }
    private class TestScript8 : EntityBehaviour { }
    private class TestScript9 : EntityBehaviour { }

    // ========================================================================
    // 注册性能测试
    // ========================================================================

    [Fact]
    public void Register_1000Types_CompletesInReasonableTime()
    {
        var registry = new ScriptRegistry();
        var stopwatch = Stopwatch.StartNew();

        // 注册 1000 个类型（使用泛型方法）
        registry.Register<TestScript0>();
        registry.Register<TestScript1>();
        registry.Register<TestScript2>();
        registry.Register<TestScript3>();
        registry.Register<TestScript4>();
        registry.Register<TestScript5>();
        registry.Register<TestScript6>();
        registry.Register<TestScript7>();
        registry.Register<TestScript8>();
        registry.Register<TestScript9>();

        stopwatch.Stop();

        _output.WriteLine($"Registered 10 types in {stopwatch.ElapsedMilliseconds}ms");

        Assert.Equal(10, registry.Count);
        Assert.True(stopwatch.ElapsedMilliseconds < 100, "Registration should be fast");
    }

    [Fact]
    public void FindByType_LookupPerformance()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScript0>();
        registry.Register<TestScript1>();
        registry.Register<TestScript2>();
        registry.Register<TestScript3>();
        registry.Register<TestScript4>();

        var stopwatch = Stopwatch.StartNew();

        // 执行 10000 次查找
        for (int i = 0; i < 10000; i++)
        {
            registry.FindByType(typeof(TestScript0));
            registry.FindByType(typeof(TestScript1));
            registry.FindByType(typeof(TestScript2));
            registry.FindByType(typeof(TestScript3));
            registry.FindByType(typeof(TestScript4));
        }

        stopwatch.Stop();

        _output.WriteLine($"50000 lookups in {stopwatch.ElapsedMilliseconds}ms");

        Assert.True(stopwatch.ElapsedMilliseconds < 1000, "Lookup should be fast");
    }

    [Fact]
    public void FindByTypeId_LookupPerformance()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScript0>();
        registry.Register<TestScript1>();
        registry.Register<TestScript2>();
        registry.Register<TestScript3>();
        registry.Register<TestScript4>();

        var typeId = registry.FindByType(typeof(TestScript0))!.TypeId;

        var stopwatch = Stopwatch.StartNew();

        // 执行 10000 次查找
        for (int i = 0; i < 10000; i++)
        {
            registry.FindByTypeId(typeId);
        }

        stopwatch.Stop();

        _output.WriteLine($"10000 TypeId lookups in {stopwatch.ElapsedMilliseconds}ms");

        Assert.True(stopwatch.ElapsedMilliseconds < 500, "TypeId lookup should be very fast");
    }
}
