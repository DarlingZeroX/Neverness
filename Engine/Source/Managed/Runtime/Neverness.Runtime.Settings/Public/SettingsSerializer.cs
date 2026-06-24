using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Neverness.Runtime.Settings;

/// <summary>
/// 纯静态序列化工具——接受 Stream 或 string，不涉及文件系统。
/// 路径由调用方决定，保持零引擎依赖。
///
/// 用法：
/// <code>
/// // 从文件加载
/// var json = File.ReadAllText("settings.json");
/// var settings = SettingsSerializer.Load&lt;GraphicsSettings&gt;(json);
///
/// // 保存到文件
/// var json = SettingsSerializer.Save(settings);
/// File.WriteAllText("settings.json", json);
///
/// // Stream 方式
/// using var stream = File.OpenRead("settings.json");
/// var settings = SettingsSerializer.Load&lt;GraphicsSettings&gt;(stream);
/// </code>
/// </summary>
public static class SettingsSerializer
{
    // ── 共用 JSON 选项 ──

    /// <summary>标准 JSON 序列化选项（CamelCase、缩进、注释跳过等）。</summary>
    public static JsonSerializerOptions DefaultOptions { get; } = CreateDefaultOptions();

    // ── 加载 ──

    /// <summary>从 JSON 字符串反序列化设置表。</summary>
    /// <typeparam name="T">设置表类型。</typeparam>
    /// <param name="json">JSON 字符串。</param>
    /// <returns>反序列化后的设置表实例。json 为空时返回默认实例。</returns>
    public static T Load<T>(string json) where T : SettingsTable, new()
    {
        var table = new T();

        if (!string.IsNullOrWhiteSpace(json))
        {
            table.LoadFromJson(json);
        }

        return table;
    }

    /// <summary>从 Stream 反序列化设置表。</summary>
    /// <typeparam name="T">设置表类型。</typeparam>
    /// <param name="stream">输入流。</param>
    /// <returns>反序列化后的设置表实例。</returns>
    public static T Load<T>(Stream stream) where T : SettingsTable, new()
    {
        using var reader = new StreamReader(stream, Encoding.UTF8);
        var json = reader.ReadToEnd();
        return Load<T>(json);
    }

    /// <summary>从 JSON 字符串反序列化为指定类型。</summary>
    /// <param name="json">JSON 字符串。</param>
    /// <param name="tableType">设置表类型。</param>
    /// <returns>反序列化后的设置表实例。</returns>
    public static SettingsTable Load(string json, Type tableType)
    {
        if (!typeof(SettingsTable).IsAssignableFrom(tableType))
            throw new ArgumentException($"类型 {tableType.Name} 不是 SettingsTable 的子类。");

        var table = (SettingsTable?)Activator.CreateInstance(tableType)
            ?? throw new InvalidOperationException($"无法创建 {tableType.Name} 的实例。");

        if (!string.IsNullOrWhiteSpace(json))
        {
            table.LoadFromJson(json);
        }

        return table;
    }

    // ── 保存 ──

    /// <summary>序列化设置表为 JSON 字符串。</summary>
    /// <typeparam name="T">设置表类型。</typeparam>
    /// <param name="settings">设置表实例。</param>
    /// <returns>格式化的 JSON 字符串。</returns>
    public static string Save<T>(T settings) where T : SettingsTable
    {
        ArgumentNullException.ThrowIfNull(settings);
        return settings.SaveToJson();
    }

    /// <summary>序列化设置表到 Stream。</summary>
    /// <typeparam name="T">设置表类型。</typeparam>
    /// <param name="settings">设置表实例。</param>
    /// <param name="stream">输出流。</param>
    public static void Save<T>(T settings, Stream stream) where T : SettingsTable
    {
        ArgumentNullException.ThrowIfNull(settings);
        ArgumentNullException.ThrowIfNull(stream);

        var json = settings.SaveToJson();
        var bytes = Encoding.UTF8.GetBytes(json);
        stream.Write(bytes, 0, bytes.Length);
    }

    // ── 工具 ──

    /// <summary>创建默认 JSON 序列化选项。</summary>
    private static JsonSerializerOptions CreateDefaultOptions()
    {
        return new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            WriteIndented = true,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
            PropertyNameCaseInsensitive = true,
            ReadCommentHandling = JsonCommentHandling.Skip,
            AllowTrailingCommas = true,
            Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
        };
    }
}
