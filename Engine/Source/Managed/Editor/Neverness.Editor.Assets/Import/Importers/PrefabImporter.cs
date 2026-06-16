using System.Text;
using System.Text.Json;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// Prefab 导入器。
///
/// 支持格式：.prefab（引擎自定义 JSON 格式）
///
/// .prefab 文件示例：
/// {
///   "name": "Enemy",
///   "root": {
///     "guid": "a1b2c3d4-...",
///     "name": "EnemyRoot",
///     "components": [
///       { "type": "Transform" },
///       { "type": "MeshRenderer", "material": "Assets/Materials/enemy.material" }
///     ],
///     "children": [
///       { "guid": "...", "name": "Weapon", "components": [...] }
///     ]
///   }
/// }
/// </summary>
[AssetImporter(".prefab")]
public class PrefabImporter : IAssetImporter
{
    public string[] SupportedExtensions => new[] { ".prefab" };
    public string DisplayName => "Prefab Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.Prefab);

            var jsonText = File.ReadAllText(context.SourceAssetPath.FullPath, Encoding.UTF8);
            var doc = JsonDocument.Parse(jsonText);
            var root = doc.RootElement;

            /* 递归收集依赖 */
            if (root.TryGetProperty("root", out var rootEntity))
                CollectDependencies(rootEntity, context);

            /* 序列化 */
            var binaryData = SerializePrefab(root);
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
            return ImportResult.Fail($"Prefab 导入异常: {ex.Message}");
        }
    }

    private void CollectDependencies(JsonElement entity, AssetImportContext context)
    {
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

        /* 递归子节点 */
        if (entity.TryGetProperty("children", out var children))
        {
            foreach (var child in children.EnumerateArray())
                CollectDependencies(child, context);
        }
    }

    private byte[] SerializePrefab(JsonElement root)
    {
        using var ms = new MemoryStream();
        using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

        /* prefab name */
        w.Write(root.TryGetProperty("name", out var name) ? (name.GetString() ?? "") : "");

        /* entity count（递归计数） */
        int entityCount = 0;
        if (root.TryGetProperty("root", out var rootEntity))
            CountEntities(rootEntity, ref entityCount);
        w.Write(entityCount);

        /* 序列化实体树 */
        if (root.TryGetProperty("root", out rootEntity))
            SerializeEntity(w, rootEntity);

        return ms.ToArray();
    }

    private void CountEntities(JsonElement entity, ref int count)
    {
        count++;
        if (entity.TryGetProperty("children", out var children))
        {
            foreach (var child in children.EnumerateArray())
                CountEntities(child, ref count);
        }
    }

    private void SerializeEntity(BinaryWriter w, JsonElement entity)
    {
        /* guid */
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

        /* name */
        w.Write(entity.TryGetProperty("name", out var name) ? (name.GetString() ?? "") : "");

        /* components */
        if (entity.TryGetProperty("components", out var comps))
        {
            w.Write(comps.GetArrayLength());
            foreach (var comp in comps.EnumerateArray())
            {
                if (comp.TryGetProperty("type", out var typeProp))
                    w.Write(typeProp.GetString() ?? "");
                else
                    w.Write("");

                var compBytes = Encoding.UTF8.GetBytes(comp.GetRawText());
                w.Write(compBytes.Length);
                w.Write(compBytes);
            }
        }
        else
        {
            w.Write(0);
        }

        /* children */
        if (entity.TryGetProperty("children", out var children))
        {
            w.Write(children.GetArrayLength());
            foreach (var child in children.EnumerateArray())
                SerializeEntity(w, child);
        }
        else
        {
            w.Write(0);
        }
    }
}
