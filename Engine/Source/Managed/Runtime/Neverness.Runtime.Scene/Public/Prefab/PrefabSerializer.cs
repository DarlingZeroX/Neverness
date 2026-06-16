using System.Numerics;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Neverness.Runtime.Scene.Prefab;

/// <summary>
/// 预制体序列化器——将预制体资产序列化为 JSON 格式。
/// </summary>
public static class PrefabSerializer
{
    private static readonly JsonSerializerOptions _options = new()
    {
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
    };

    /// <summary>序列化预制体为 JSON 字符串。</summary>
    public static string Serialize(PrefabAsset prefab)
    {
        ArgumentNullException.ThrowIfNull(prefab);
        return JsonSerializer.Serialize(prefab, _options);
    }

    /// <summary>序列化预制体到流。</summary>
    public static void SerializeToStream(PrefabAsset prefab, Stream stream)
    {
        ArgumentNullException.ThrowIfNull(prefab);
        ArgumentNullException.ThrowIfNull(stream);
        JsonSerializer.Serialize(stream, prefab, _options);
    }

    /// <summary>从 JSON 字符串反序列化预制体。</summary>
    public static PrefabAsset? Deserialize(string json)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(json);
        return JsonSerializer.Deserialize<PrefabAsset>(json, _options);
    }

    /// <summary>从流反序列化预制体。</summary>
    public static PrefabAsset? DeserializeFromStream(Stream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        return JsonSerializer.Deserialize<PrefabAsset>(stream, _options);
    }

    /// <summary>保存预制体到文件。</summary>
    public static void SaveToFile(PrefabAsset prefab, string filePath)
    {
        ArgumentNullException.ThrowIfNull(prefab);
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);

        var json = Serialize(prefab);
        File.WriteAllText(filePath, json);
    }

    /// <summary>从文件加载预制体。</summary>
    public static PrefabAsset? LoadFromFile(string filePath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);

        if (!File.Exists(filePath))
            return null;

        var json = File.ReadAllText(filePath);
        return Deserialize(json);
    }
}
