using Neverness.Runtime.Settings;
using Xunit;

namespace Neverness.Runtime.Settings.Tests;

/// <summary>
/// FieldDescriptor 和 FieldType 测试。
/// </summary>
public sealed class FieldDescriptorTests
{
    [Fact]
    public void FieldDescriptor_Creation()
    {
        var fd = new FieldDescriptor
        {
            Name = "Test",
            DisplayName = "测试",
            FieldType = FieldType.Int,
            ValueType = typeof(int),
            Min = 0,
            Max = 100,
        };

        Assert.Equal("Test", fd.Name);
        Assert.Equal("测试", fd.DisplayName);
        Assert.Equal(FieldType.Int, fd.FieldType);
        Assert.Equal(typeof(int), fd.ValueType);
        Assert.Equal(0, fd.Min);
        Assert.Equal(100, fd.Max);
        Assert.True(fd.HasRange);
    }

    [Fact]
    public void FieldDescriptor_NoRange()
    {
        var fd = new FieldDescriptor
        {
            Name = "X",
            DisplayName = "X",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
        };

        Assert.False(fd.HasRange);
    }

    [Fact]
    public void FieldDescriptor_GetterSetter_Work()
    {
        var table = new TestSettings();
        var fd = new FieldDescriptor
        {
            Name = "IntProp",
            DisplayName = "整数",
            FieldType = FieldType.Int,
            ValueType = typeof(int),
            Getter = t => ((TestSettings)t).IntProp,
            Setter = (t, v) => ((TestSettings)t).IntProp = (int)v!,
        };

        Assert.Equal(42, fd.Getter!(table));
        fd.Setter!(table, 99);
        Assert.Equal(99, fd.Getter(table));
    }

    [Fact]
    public void FieldDescriptor_EnumType()
    {
        var fd = new FieldDescriptor
        {
            Name = "EnumProp",
            DisplayName = "枚举",
            FieldType = FieldType.Enum,
            ValueType = typeof(TestEnum),
            EnumType = typeof(TestEnum),
            EnumValues = Enum.GetValues<TestEnum>(),
            EnumDisplayNames = Enum.GetNames<TestEnum>(),
        };

        Assert.Equal(3, fd.EnumValues!.Length);
        Assert.Equal(3, fd.EnumDisplayNames!.Length);
        Assert.Equal("First", fd.EnumDisplayNames[0]);
        Assert.Equal("Second", fd.EnumDisplayNames[1]);
        Assert.Equal("Third", fd.EnumDisplayNames[2]);
    }

    [Fact]
    public void FieldType_AllValues_AreDefined()
    {
        // 确保枚举值连续，没有意外添加的
        var values = Enum.GetValues<FieldType>();
        Assert.Equal(10, values.Length); // Bool..Custom
    }
}
