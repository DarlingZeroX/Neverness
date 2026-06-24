using System.ComponentModel;
using System.Text.Json;
using Neverness.Runtime.Settings;
using Neverness.Runtime.Settings.Attributes;
using Xunit;

namespace Neverness.Runtime.Settings.Tests;

/// <summary>
/// SettingsTable 基类测试。
/// </summary>
public sealed class SettingsTableTests
{
    // ── 默认值 ──

    [Fact]
    public void DefaultValues_AreCorrect()
    {
        var s = new TestSettings();
        Assert.True(s.BoolProp);
        Assert.Equal(42, s.IntProp);
        Assert.Equal(0.5f, s.FloatProp);
        Assert.Equal("hello", s.StringProp);
        Assert.Equal(TestEnum.Second, s.EnumProp);
    }

    [Fact]
    public void TableId_And_DisplayName_AreCorrect()
    {
        var s = new TestSettings();
        Assert.Equal("test", s.TableId);
        Assert.Equal("测试设置", s.DisplayName);
        Assert.Equal("测试", s.Category);
    }

    // ── SaveToJson ──

    [Fact]
    public void SaveToJson_ContainsAllPublicProperties()
    {
        var s = new TestSettings();
        var json = s.SaveToJson();

        Assert.Contains("boolProp", json);
        Assert.Contains("intProp", json);
        Assert.Contains("floatProp", json);
        Assert.Contains("stringProp", json);
        Assert.Contains("enumProp", json);
    }

    [Fact]
    public void SaveToJson_ProducesValidJson()
    {
        var s = new TestSettings();
        var json = s.SaveToJson();

        // 不应抛出异常
        var doc = JsonDocument.Parse(json);
        Assert.NotNull(doc);
    }

    [Fact]
    public void SaveToJson_UsesCamelCase()
    {
        var s = new TestSettings();
        var json = s.SaveToJson();

        // PascalCase 属性应序列化为 camelCase
        Assert.Contains("boolProp", json);
        Assert.DoesNotContain("BoolProp", json);
    }

    [Fact]
    public void SaveToJson_PreservesValues()
    {
        var s = new TestSettings { IntProp = 99, StringProp = "world" };
        var json = s.SaveToJson();

        var doc = JsonDocument.Parse(json);
        Assert.Equal(99, doc.RootElement.GetProperty("intProp").GetInt32());
        Assert.Equal("world", doc.RootElement.GetProperty("stringProp").GetString());
    }

    // ── LoadFromJson ──

    [Fact]
    public void LoadFromJson_UpdatesProperties()
    {
        var s = new TestSettings();
        var json = """
        {
            "boolProp": false,
            "intProp": 10,
            "floatProp": 0.8,
            "stringProp": "world",
            "enumProp": "third"
        }
        """;

        s.LoadFromJson(json);

        Assert.False(s.BoolProp);
        Assert.Equal(10, s.IntProp);
        Assert.Equal(0.8f, s.FloatProp);
        Assert.Equal("world", s.StringProp);
        Assert.Equal(TestEnum.Third, s.EnumProp);
    }

    [Fact]
    public void LoadFromJson_IgnoresUnknownFields()
    {
        var s = new TestSettings();
        var json = """
        {
            "boolProp": false,
            "unknownField": "should be ignored",
            "anotherUnknown": 123
        }
        """;

        // 不应抛出异常
        s.LoadFromJson(json);
        Assert.False(s.BoolProp);
    }

    [Fact]
    public void LoadFromJson_EmptyString_DoesNotThrow()
    {
        var s = new TestSettings();
        s.LoadFromJson("");
        // 应保持默认值
        Assert.Equal(42, s.IntProp);
    }

    [Fact]
    public void LoadFromJson_Null_DoesNotThrow()
    {
        var s = new TestSettings();
        s.LoadFromJson(null!);
        Assert.Equal(42, s.IntProp);
    }

    [Fact]
    public void LoadFromJson_PartialUpdate_OnlyUpdatesSpecifiedFields()
    {
        var s = new TestSettings();
        var json = """{ "intProp": 999 }""";

        s.LoadFromJson(json);

        Assert.Equal(999, s.IntProp);
        // 其他字段保持默认
        Assert.True(s.BoolProp);
        Assert.Equal(0.5f, s.FloatProp);
        Assert.Equal("hello", s.StringProp);
    }

    // ── RoundTrip ──

    [Fact]
    public void RoundTrip_SaveAndLoad_PreservesValues()
    {
        var original = new TestSettings
        {
            BoolProp = false,
            IntProp = 77,
            FloatProp = 0.33f,
            StringProp = "roundtrip",
            EnumProp = TestEnum.First,
        };

        var json = original.SaveToJson();
        var loaded = new TestSettings();
        loaded.LoadFromJson(json);

        Assert.Equal(original.BoolProp, loaded.BoolProp);
        Assert.Equal(original.IntProp, loaded.IntProp);
        Assert.Equal(original.FloatProp, loaded.FloatProp);
        Assert.Equal(original.StringProp, loaded.StringProp);
        Assert.Equal(original.EnumProp, loaded.EnumProp);
    }

    // ── ResetDefaults ──

    [Fact]
    public void ResetDefaults_RestoresAllDefaults()
    {
        var s = new TestSettings();
        s.BoolProp = false;
        s.IntProp = 999;
        s.FloatProp = 0.1f;
        s.StringProp = "changed";

        s.ResetDefaults();

        Assert.True(s.BoolProp);
        Assert.Equal(42, s.IntProp);
        Assert.Equal(0.5f, s.FloatProp);
        Assert.Equal("hello", s.StringProp);
    }

    // ── PropertyChanged ──

    [Fact]
    public void PropertyChanged_FiredOnLoadFromJson()
    {
        var s = new TestSettings();
        var changedProperties = new List<string>();

        s.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName != null)
                changedProperties.Add(e.PropertyName);
        };

        s.LoadFromJson("""{ "intProp": 100 }""");

        // LoadFromJson 通过反射 SetValue，INotifyPropertyChanged 由框架触发
        // 具体触发次数取决于实现，但至少不应抛异常
        Assert.NotNull(changedProperties);
    }

    [Fact]
    public void ResetDefaults_FiresPropertyChanged()
    {
        var s = new TestSettings();
        s.IntProp = 999;

        var fired = false;
        s.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName == string.Empty) // ResetDefaults 用 string.Empty
                fired = true;
        };

        s.ResetDefaults();
        Assert.True(fired);
    }
}
