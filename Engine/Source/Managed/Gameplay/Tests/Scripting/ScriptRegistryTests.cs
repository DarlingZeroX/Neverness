// ============================================================================
// ScriptRegistryTests.cs - ScriptRegistry 单元测试
// ============================================================================

using Neverness.Gameplay;
using Xunit;

namespace Neverness.Gameplay.Tests.Scripting;

/// <summary>
/// ScriptRegistry 单元测试。
/// </summary>
public class ScriptRegistryTests
{
    // ========================================================================
    // 测试用脚本类
    // ========================================================================

    private class TestScriptA : EntityBehaviour { }
    private class TestScriptB : EntityBehaviour { }
    private abstract class AbstractScript : EntityBehaviour { }

    // ========================================================================
    // 注册测试
    // ========================================================================

    [Fact]
    public void Register_AddsTypeToRegistry()
    {
        var registry = new ScriptRegistry();

        registry.Register<TestScriptA>();

        Assert.Equal(1, registry.Count);
        Assert.True(registry.IsRegistered(typeof(TestScriptA)));
    }

    [Fact]
    public void Register_MultipleTypes_AddsAllTypes()
    {
        var registry = new ScriptRegistry();

        registry.Register<TestScriptA>();
        registry.Register<TestScriptB>();

        Assert.Equal(2, registry.Count);
        Assert.True(registry.IsRegistered(typeof(TestScriptA)));
        Assert.True(registry.IsRegistered(typeof(TestScriptB)));
    }

    [Fact]
    public void Register_SameTypeTwice_DoesNotDuplicate()
    {
        var registry = new ScriptRegistry();

        registry.Register<TestScriptA>();
        registry.Register<TestScriptA>();

        Assert.Equal(1, registry.Count);
    }

    // ========================================================================
    // 查找测试
    // ========================================================================

    [Fact]
    public void FindByType_ReturnsCorrectInfo()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScriptA>();

        var info = registry.FindByType(typeof(TestScriptA));

        Assert.NotNull(info);
        Assert.Equal("TestScriptA", info.Name);
    }

    [Fact]
    public void FindByType_UnknownType_ReturnsNull()
    {
        var registry = new ScriptRegistry();

        var info = registry.FindByType(typeof(TestScriptA));

        Assert.Null(info);
    }

    [Fact]
    public void FindByName_ReturnsCorrectInfo()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScriptA>();

        var info = registry.FindByName(typeof(TestScriptA).FullName!);

        Assert.NotNull(info);
        Assert.Equal("TestScriptA", info.Name);
    }

    [Fact]
    public void FindByTypeId_ReturnsCorrectInfo()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScriptA>();

        var info = registry.FindByType(typeof(TestScriptA));
        Assert.NotNull(info);

        var found = registry.FindByTypeId(info.TypeId);

        Assert.NotNull(found);
        Assert.Equal(info.TypeId, found.TypeId);
    }

    [Fact]
    public void GetAllScripts_ReturnsAllRegisteredTypes()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScriptA>();
        registry.Register<TestScriptB>();

        var all = registry.GetAllScripts();

        Assert.Equal(2, all.Count);
    }

    // ========================================================================
    // 清理测试
    // ========================================================================

    [Fact]
    public void Clear_RemovesAllRegistrations()
    {
        var registry = new ScriptRegistry();
        registry.Register<TestScriptA>();
        registry.Register<TestScriptB>();

        registry.Clear();

        Assert.Equal(0, registry.Count);
        Assert.False(registry.IsRegistered(typeof(TestScriptA)));
    }
}
