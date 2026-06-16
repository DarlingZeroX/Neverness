using System.Text;
using System.Text.Json;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 场景导入器。
///
/// 支持格式：.scene（引擎自定义 JSON 格式）
///
/// .scene 文件示例：
/// {
///   "name": "MainScene",
///   "entities": [
///     {
///       "guid": "7bfa3c01-...",
///       "name": "Camera",
///       "parent": null,
///       "components": [
///         { "type": "Transform", "position": [0, 1, -5] },
///         { "type": "Camera", "fov": 60 }
///       ]
///     }
///   ],
///   "settings": {
///     "skybox": "Assets/Textures/sky.hdr",
///     "ambientColor": [0.2, 0.2, 0.2]
///   }
/// }
/// </summary>
[AssetImporter(".scene")]
public class SceneImporter : IAssetImporter
{
    public string[] SupportedExtensions => new[] { ".scene" };
    public string DisplayName => "Scene Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.Scene);

            var jsonText = File.ReadAllText(context.SourceAssetPath.FullPath, Encoding.UTF8);
            var doc = JsonDocument.Parse(jsonText);
            var root = doc.RootElement;

            /* 收集 prefab/材质/纹理依赖 */
            if (root.TryGetProperty("entities", out var entities))
            {
                foreach (var entity in entities.EnumerateArray())
                {
                    /* 依赖 prefab */
                    if (entity.TryGetProperty("prefab", out var prefabRef))
                    {
                        var path = prefabRef.GetString();
                        if (!string.IsNullOrEmpty(path))
                            context.Dependencies.AddByPath(path);
                    }

                    /* 组件引用的资产 */
                    if (entity.TryGetProperty("components", out var components))
                    {
                        foreach (var comp in components.EnumerateArray())
                        {
                            foreach (var prop in comp.EnumerateObject())
                            {
                                if (prop.Value.ValueKind == JsonValueKind.String)
                                {
                                    var val = prop.Value.GetString();
                                    if (!string.IsNullOrEmpty(val) && val.Contains('/'))
                                        context.Dependencies.AddByPath(val);
                                }
                            }
                        }
                    }
                }
            }

            /* 序列化为二进制 */
            var binaryData = SerializeScene(root);
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.EntityHierarchy,
                Data = binaryData
            });

            result.Dependencies.AddRange(context.Dependencies.Dependencies);

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"场景导入异常: {ex.Message}");
        }
    }

    private byte[] SerializeScene(JsonElement root)
    {
        using var ms = new MemoryStream();
        using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

        /* scene name */
        w.Write(root.TryGetProperty("name", out var name) ? (name.GetString() ?? string.Empty) : string.Empty);

        /* entities */
        if (root.TryGetProperty("entities", out var entities))
        {
            w.Write(entities.GetArrayLength());
            foreach (var entity in entities.EnumerateArray())
            {
                /* entity guid */
                if (entity.TryGetProperty("guid", out var guidProp))
                {
                    var guid = GUID.Parse(guidProp.GetString() ?? "");
                    w.Write(guid.High);
                    w.Write(guid.Low);
                }
                else
                {
                    w.Write(0UL);
                    w.Write(0UL);
                }

                /* entity name */
                w.Write(entity.TryGetProperty("name", out var en) ? (en.GetString() ?? "") : "");

                /* parent guid (nullable) */
                if (entity.TryGetProperty("parent", out var parent) && parent.ValueKind == JsonValueKind.String)
                {
                    var pg = GUID.Parse(parent.GetString() ?? "");
                    w.Write(true);
                    w.Write(pg.High);
                    w.Write(pg.Low);
                }
                else
                {
                    w.Write(false);
                }

                /* components */
                if (entity.TryGetProperty("components", out var comps))
                {
                    w.Write(comps.GetArrayLength());
                    foreach (var comp in comps.EnumerateObject())
                    {
                        w.Write(comp.Name);
                        /* 序列化组件属性为 JSON 字节 */
                        var compBytes = Encoding.UTF8.GetBytes(comp.Value.GetRawText());
                        w.Write(compBytes.Length);
                        w.Write(compBytes);
                    }
                }
                else
                {
                    w.Write(0);
                }
            }
        }
        else
        {
            w.Write(0);
        }

        return ms.ToArray();
    }
}
