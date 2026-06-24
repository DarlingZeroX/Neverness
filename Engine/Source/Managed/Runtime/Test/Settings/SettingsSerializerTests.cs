using System.Text;
using Neverness.Runtime.Settings;
using Xunit;

namespace Neverness.Runtime.Settings.Tests;

/// <summary>
/// SettingsSerializer 静态工具测试。
/// </summary>
public sealed class SettingsSerializerTests
{
    // ── Load&lt;T&gt;(string) ──

    [Fact]
    public void Load_FromString_ReturnsDefaultWhenEmpty()
    {
        var s = SettingsSerializer.Load<TestSettings>("");
        Assert.NotNull(s);
        Assert.Equal(42, s.IntProp);
    }

    [Fact]
    public void Load_FromString_ReturnsDefaultWhenNull()
    {
        var s = SettingsSerializer.Load<TestSettings>((string)null!);
        Assert.NotNull(s);
        Assert.Equal(42, s.IntProp);
    }

    [Fact]
    public void Load_FromString_DeserializesValues()
    {
        var json = """{ "intProp": 10, "stringProp": "loaded" }""";
        var s = SettingsSerializer.Load<TestSettings>(json);

        Assert.Equal(10, s.IntProp);
        Assert.Equal("loaded", s.StringProp);
    }

    // ── Load&lt;T&gt;(Stream) ──

    [Fact]
    public void Load_FromStream_DeserializesValues()
    {
        var json = """{ "intProp": 20, "boolProp": false }""";
        var bytes = Encoding.UTF8.GetBytes(json);
        using var stream = new MemoryStream(bytes);

        var s = SettingsSerializer.Load<TestSettings>(stream);

        Assert.Equal(20, s.IntProp);
        Assert.False(s.BoolProp);
    }

    [Fact]
    public void Load_FromStream_EmptyStream_ReturnsDefault()
    {
        using var stream = new MemoryStream();
        var s = SettingsSerializer.Load<TestSettings>(stream);

        Assert.NotNull(s);
        Assert.Equal(42, s.IntProp);
    }

    // ── Load(string, Type) ──

    [Fact]
    public void Load_WithType_DeserializesValues()
    {
        var json = """{ "intProp": 30 }""";
        var s = SettingsSerializer.Load(json, typeof(TestSettings));

        Assert.IsType<TestSettings>(s);
        Assert.Equal(30, ((TestSettings)s).IntProp);
    }

    [Fact]
    public void Load_WithType_ThrowsForNonSettingsTableType()
    {
        Assert.Throws<ArgumentException>(() =>
            SettingsSerializer.Load("{}", typeof(string)));
    }

    // ── Save&lt;T&gt; ──

    [Fact]
    public void Save_ToString_ProducesValidJson()
    {
        var s = new TestSettings { IntProp = 55 };
        var json = SettingsSerializer.Save(s);

        Assert.Contains("55", json);
        Assert.Contains("intProp", json);
    }

    [Fact]
    public void Save_ToStream_WritesCorrectBytes()
    {
        var s = new TestSettings { IntProp = 66 };
        using var stream = new MemoryStream();

        SettingsSerializer.Save(s, stream);

        stream.Position = 0;
        var json = Encoding.UTF8.GetString(stream.ToArray());
        Assert.Contains("66", json);
    }

    [Fact]
    public void Save_ThrowsOnNull()
    {
        Assert.Throws<ArgumentNullException>(() =>
            SettingsSerializer.Save<TestSettings>(null!));
    }

    // ── RoundTrip ──

    [Fact]
    public void RoundTrip_SaveAndLoad_ViaSerializer()
    {
        var original = new TestSettings
        {
            BoolProp = false,
            IntProp = 88,
            FloatProp = 0.75f,
            StringProp = "serializer",
            EnumProp = TestEnum.Third,
        };

        var json = SettingsSerializer.Save(original);
        var loaded = SettingsSerializer.Load<TestSettings>(json);

        Assert.Equal(original.BoolProp, loaded.BoolProp);
        Assert.Equal(original.IntProp, loaded.IntProp);
        Assert.Equal(original.FloatProp, loaded.FloatProp);
        Assert.Equal(original.StringProp, loaded.StringProp);
        Assert.Equal(original.EnumProp, loaded.EnumProp);
    }

    [Fact]
    public void RoundTrip_ViaStream()
    {
        var original = new MinimalSettings { Value = 123 };
        using var stream = new MemoryStream();

        SettingsSerializer.Save(original, stream);
        stream.Position = 0;
        var loaded = SettingsSerializer.Load<MinimalSettings>(stream);

        Assert.Equal(123, loaded.Value);
    }

    // ── DefaultOptions ──

    [Fact]
    public void DefaultOptions_IsNotNull()
    {
        Assert.NotNull(SettingsSerializer.DefaultOptions);
    }

    [Fact]
    public void DefaultOptions_UsesCamelCase()
    {
        Assert.NotNull(SettingsSerializer.DefaultOptions.PropertyNamingPolicy);
    }
}
