using System.Globalization;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// .meta 檔案管理器。
///
/// 每個 source asset 對應一個同名 .meta 側車檔案：
///   hero.png ↔ hero.png.meta
///
/// .meta 格式（YAML 子集）：
///   guid: 7bfa3c01-4e8a-4b2d-9f1e-a1b2c3d4e5f6
///   importer: TextureImporter
///   importSettings:
///     compression: BC7
///     generateMipmaps: true
///   labels:
///     - character
///   dependencies: []
///
/// 設計原則：
///   - Editor-only，Runtime 不讀 .meta
///   - GUID 一旦建立永久不變
///   - source asset rename/move 時 .meta 跟隨移動
/// </summary>
public static class MetaFileManager
{
    /* 舊版 [VGASSET] 格式的前綴 */
    private const string LegacyPrefix = "[VGASSET];";

    /* ======================== 公開 API ======================== */

    /// <summary>取得資產的 .meta 檔案路徑。</summary>
    public static NPath GetMetaPath(NPath assetPath) => new(assetPath.FullPath + ".meta");

    /// <summary>.meta 檔案是否存在。</summary>
    public static bool MetaExists(NPath assetPath) => File.Exists(GetMetaPath(assetPath).FullPath);

    /// <summary>讀取 .meta，回傳 AssetMeta。不存在回傳 null。</summary>
    public static AssetMeta? ReadMeta(NPath assetPath)
    {
        var metaPath = GetMetaPath(assetPath);
        if (!File.Exists(metaPath.FullPath))
            return null;

        try
        {
            var content = File.ReadAllText(metaPath.FullPath);

            /* 嘗試舊格式遷移 */
            if (content.StartsWith(LegacyPrefix, StringComparison.Ordinal))
                return MigrateFromLegacy(content, assetPath);

            /* 解析 YAML 子集 */
            return ParseYaml(content);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[MetaFileManager] 讀取 .meta 失敗: {metaPath} → {ex.Message}");
            return null;
        }
    }

    /// <summary>建立新的 .meta 檔案（自動生成 GUID）。</summary>
    public static AssetMeta CreateMeta(NPath assetPath, string importerName)
    {
        var meta = new AssetMeta
        {
            Guid = GUID.NewRandom(),
            Importer = importerName,
            ImportSettings = new Dictionary<string, string>(),
            Labels = new List<string>(),
            Dependencies = new List<string>()
        };

        WriteMeta(assetPath, meta);
        return meta;
    }

    /// <summary>取得或建立 .meta（不存在則自動建立）。</summary>
    public static AssetMeta GetOrCreateMeta(NPath assetPath, string importerName)
    {
        return ReadMeta(assetPath) ?? CreateMeta(assetPath, importerName);
    }

    /// <summary>寫入 .meta 檔案。</summary>
    public static void WriteMeta(NPath assetPath, AssetMeta meta)
    {
        var metaPath = GetMetaPath(assetPath);

        try
        {
            var dir = Path.GetDirectoryName(metaPath.FullPath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            var content = SerializeToYaml(meta);
            File.WriteAllText(metaPath.FullPath, content);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[MetaFileManager] 寫入 .meta 失敗: {metaPath} → {ex.Message}");
        }
    }

    /// <summary>移動 .meta 檔案（隨 source asset 一起移動）。</summary>
    public static void MoveMeta(NPath fromAssetPath, NPath toAssetPath)
    {
        var fromMeta = GetMetaPath(fromAssetPath);
        var toMeta = GetMetaPath(toAssetPath);

        if (File.Exists(fromMeta.FullPath))
        {
            try
            {
                var dir = Path.GetDirectoryName(toMeta.FullPath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                File.Move(fromMeta.FullPath, toMeta.FullPath, overwrite: true);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[MetaFileManager] 移動 .meta 失敗: {fromMeta} → {toMeta} → {ex.Message}");
            }
        }
    }

    /// <summary>刪除 .meta 檔案。</summary>
    public static void DeleteMeta(NPath assetPath)
    {
        var metaPath = GetMetaPath(assetPath);
        if (File.Exists(metaPath.FullPath))
        {
            try { File.Delete(metaPath.FullPath); }
            catch { /* 忽略 */ }
        }
    }

    /// <summary>依擴展名推斷預設 importer 名稱。</summary>
    public static string InferImporterName(string extension)
    {
        return extension.ToLowerInvariant() switch
        {
            ".png" or ".jpg" or ".jpeg" or ".tga" or ".bmp" or ".dds" or ".hdr" => "TextureImporter",
            ".fbx" or ".obj" or ".gltf" or ".glb" => "MeshImporter",
            ".wav" or ".ogg" or ".mp3" or ".flac" => "AudioImporter",
            ".material" => "MaterialImporter",
            ".hlsl" or ".glsl" or ".nnshader" => "ShaderImporter",
            ".scene" => "SceneImporter",
            ".prefab" => "PrefabImporter",
            ".lua" => "LuaScriptImporter",
            ".anim" => "AnimationImporter",
            ".cs" => "ScriptAssetImporter",
            _ => "DefaultImporter"
        };
    }

    /* ======================== YAML 序列化（輕量實作） ======================== */

    private static string SerializeToYaml(AssetMeta meta)
    {
        var sb = new System.Text.StringBuilder();

        sb.AppendLine($"guid: {meta.Guid.ToUuidString()}");
        sb.AppendLine($"importer: {meta.Importer}");

        /* importSettings */
        if (meta.ImportSettings is { Count: > 0 })
        {
            sb.AppendLine("importSettings:");
            foreach (var (key, value) in meta.ImportSettings)
            {
                if (value.Contains('\n'))
                {
                    sb.AppendLine($"  {key}: |");
                    foreach (var line in value.Split('\n'))
                        sb.AppendLine($"    {line}");
                }
                else
                {
                    sb.AppendLine($"  {key}: {EscapeYamlValue(value)}");
                }
            }
        }

        /* labels */
        if (meta.Labels is { Count: > 0 })
        {
            sb.AppendLine("labels:");
            foreach (var label in meta.Labels)
                sb.AppendLine($"  - {label}");
        }

        /* dependencies */
        if (meta.Dependencies is { Count: > 0 })
        {
            sb.AppendLine("dependencies:");
            foreach (var dep in meta.Dependencies)
                sb.AppendLine($"  - {dep}");
        }

        return sb.ToString();
    }

    private static AssetMeta ParseYaml(string content)
    {
        var meta = new AssetMeta();
        var lines = content.Split('\n');

        string? currentSection = null;

        foreach (var rawLine in lines)
        {
            var line = rawLine.TrimEnd('\r');

            if (string.IsNullOrWhiteSpace(line) || line.StartsWith('#'))
                continue;

            if (!line.StartsWith("  ", StringComparison.Ordinal) && !line.StartsWith('\t'))
            {
                var colonIdx = line.IndexOf(':');
                if (colonIdx > 0)
                {
                    var key = line[..colonIdx].Trim();
                    var value = line[(colonIdx + 1)..].Trim();

                    currentSection = key;

                    switch (key)
                    {
                        case "guid":
                            meta.Guid = ParseGuid(value);
                            break;
                        case "importer":
                            meta.Importer = value;
                            break;
                        case "importSettings":
                            meta.ImportSettings ??= new Dictionary<string, string>();
                            break;
                        case "labels":
                            meta.Labels ??= new List<string>();
                            break;
                        case "dependencies":
                            meta.Dependencies ??= new List<string>();
                            break;
                    }
                }
            }
            else
            {
                var trimmed = line.Trim();

                if (currentSection == "importSettings")
                {
                    meta.ImportSettings ??= new Dictionary<string, string>();
                    var colonIdx = trimmed.IndexOf(':');
                    if (colonIdx > 0)
                    {
                        var k = trimmed[..colonIdx].Trim();
                        var v = trimmed[(colonIdx + 1)..].Trim();
                        meta.ImportSettings[k] = UnescapeYamlValue(v);
                    }
                }
                else if (currentSection == "labels" && trimmed.StartsWith("- "))
                {
                    meta.Labels ??= new List<string>();
                    meta.Labels.Add(trimmed[2..].Trim());
                }
                else if (currentSection == "dependencies" && trimmed.StartsWith("- "))
                {
                    meta.Dependencies ??= new List<string>();
                    meta.Dependencies.Add(trimmed[2..].Trim());
                }
            }
        }

        return meta;
    }

    private static GUID ParseGuid(string value)
    {
        if (value.Length == 36 && value[8] == '-' && value[13] == '-')
        {
            var hex = value.Replace("-", "");
            return GUID.Parse(hex);
        }

        if (value.Length == 32)
            return GUID.Parse(value);

        return GUID.Zero;
    }

    private static string EscapeYamlValue(string value)
    {
        if (value.Contains(':') || value.Contains('#') || value.Contains('"'))
            return $"\"{value.Replace("\"", "\\\"")}\"";
        return value;
    }

    private static string UnescapeYamlValue(string value)
    {
        if (value.StartsWith('"') && value.EndsWith('"'))
            return value[1..^1].Replace("\\\"", "\"");
        return value;
    }

    /* ======================== 舊格式遷移 ======================== */

    private static AssetMeta MigrateFromLegacy(string content, NPath assetPath)
    {
        var parts = content.Split('\n')[0].Split(';');
        var meta = new AssetMeta
        {
            Guid = GUID.NewRandom(),
            Importer = InferImporterName(assetPath.Extension),
            ImportSettings = new Dictionary<string, string>(),
            Labels = new List<string>(),
            Dependencies = new List<string>()
        };

        if (parts.Length >= 3)
        {
            var legacyIdStr = parts[1];
            if (ulong.TryParse(legacyIdStr, CultureInfo.InvariantCulture, out var legacyId))
            {
                meta.ImportSettings["_legacyId"] = legacyIdStr;
            }

            meta.ImportSettings["_legacyType"] = parts[2].Trim();
        }

        WriteMeta(assetPath, meta);

        Console.WriteLine($"[MetaFileManager] 遷移舊格式 .meta: {assetPath} → GUID={meta.Guid.ToUuidString()}");

        return meta;
    }
}

/// <summary>
/// 資產 .meta 資料模型。
/// </summary>
public sealed class AssetMeta
{
    /// <summary>128-bit 資產 GUID（UUID v4）。</summary>
    public GUID Guid { get; set; }

    /// <summary>Importer 名稱（如 TextureImporter）。</summary>
    public string Importer { get; set; } = "DefaultImporter";

    /// <summary>Importer 特有設定（key-value）。</summary>
    public Dictionary<string, string> ImportSettings { get; set; } = new();

    /// <summary>Addressable 標籤列表。</summary>
    public List<string> Labels { get; set; } = new();

    /// <summary>顯式依賴 GUID 列表（hex 字串）。</summary>
    public List<string> Dependencies { get; set; } = new();

    /// <summary>資產類型 ID（與 AssetTypeId 常量對齊）。</summary>
    public ulong AssetTypeId { get; set; }

    /// <summary>是否為新建（GUID 尚未持久化）。</summary>
    public bool IsNew => Guid.IsZero;

    /// <summary>依擴展名推斷資產類型 ID。</summary>
    public static ulong InferAssetTypeId(string extension)
    {
        return extension.ToLowerInvariant() switch
        {
            ".png" or ".jpg" or ".jpeg" or ".tga" or ".bmp" or ".dds" or ".hdr" => 1, // Texture2D
            ".fbx" or ".obj" or ".gltf" or ".glb" => 2, // Mesh
            ".wav" or ".ogg" or ".mp3" or ".flac" => 3, // AudioClip
            ".material" => 4,  // Material
            ".hlsl" or ".glsl" or ".nnshader" => 5, // Shader
            ".scene" => 6,     // Scene
            ".prefab" => 7,    // Prefab
            ".anim" => 8,      // Animation
            ".lua" => 9,       // LuaScript
            ".cs" => 11,       // CSharpScript
            ".html" or ".htm" or ".rml" or ".css" or ".rcss" or ".js" => 12, // HtmlDocument
            _ => 0
        };
    }
}
