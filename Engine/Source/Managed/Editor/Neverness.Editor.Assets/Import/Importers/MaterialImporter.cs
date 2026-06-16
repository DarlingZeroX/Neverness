using System.Text;
using System.Text.Json;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 材质导入器。
///
/// 支持格式：.material（引擎自定义 JSON 格式）
///
/// .material 文件示例：
/// {
///   "shader": "Assets/Shaders/default_lit.nnshader",
///   "properties": {
///     "albedoColor": [1, 1, 1, 1],
///     "metallic": 0.0,
///     "roughness": 0.5,
///     "albedoTexture": "Assets/Textures/default_albedo.png"
///   },
///   "renderQueue": 2000,
///   "keywords": ["ALBEDO_MAP"]
/// }
///
/// 导入设置：
///   validateShader — true | false（默认 true，检查 shader 是否存在）
/// </summary>
[AssetImporter(".material")]
public class MaterialImporter : IAssetImporter
{
    public string[] SupportedExtensions => new[] { ".material" };
    public string DisplayName => "Material Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.Material);

            var jsonText = File.ReadAllText(context.SourceAssetPath.FullPath, Encoding.UTF8);
            var doc = JsonDocument.Parse(jsonText);
            var root = doc.RootElement;

            /* 收集 shader 依赖 */
            if (root.TryGetProperty("shader", out var shaderProp))
            {
                var shaderPath = shaderProp.GetString();
                if (!string.IsNullOrEmpty(shaderPath))
                    context.Dependencies.AddByPath(shaderPath);
            }

            /* 收集 texture 依赖 */
            if (root.TryGetProperty("properties", out var props))
            {
                foreach (var prop in props.EnumerateObject())
                {
                    if (prop.Value.ValueKind == JsonValueKind.String)
                    {
                        var path = prop.Value.GetString();
                        if (!string.IsNullOrEmpty(path)
                            && (path.StartsWith("Assets/", StringComparison.OrdinalIgnoreCase)
                                || path.Contains('/')))
                        {
                            context.Dependencies.AddByPath(path);
                        }
                    }
                }
            }

            /* 序列化为二进制 blob */
            var binaryData = SerializeMaterial(root);
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.Data,
                Data = binaryData
            });

            result.Dependencies.AddRange(context.Dependencies.Dependencies);

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"材质导入异常: {ex.Message}");
        }
    }

    /// <summary>
    /// 简单二进制序列化（替代 JSON 运行时解析）。
    ///
    /// 格式：
    ///   [renderQueue: uint32]
    ///   [keywordCount: uint32]
    ///   [keywords: UTF8 字符串数组]
    ///   [propertyCount: uint32]
    ///   [properties: (name, type, value) 元组数组]
    /// </summary>
    private byte[] SerializeMaterial(JsonElement root)
    {
        using var ms = new MemoryStream();
        using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

        /* renderQueue */
        uint renderQueue = 2000;
        if (root.TryGetProperty("renderQueue", out var rq))
            renderQueue = (uint)rq.GetInt32();
        w.Write(renderQueue);

        /* keywords */
        if (root.TryGetProperty("keywords", out var kwArr) && kwArr.ValueKind == JsonValueKind.Array)
        {
            w.Write(kwArr.GetArrayLength());
            foreach (var kw in kwArr.EnumerateArray())
                w.Write(kw.GetString() ?? string.Empty);
        }
        else
        {
            w.Write(0);
        }

        /* properties */
        if (root.TryGetProperty("properties", out var props))
        {
            var propCount = props.EnumerateObject().Count();
            w.Write(propCount);

            foreach (var prop in props.EnumerateObject())
            {
                w.Write(prop.Name);

                switch (prop.Value.ValueKind)
                {
                    case JsonValueKind.Number:
                        w.Write((byte)1); /* float */
                        w.Write(prop.Value.GetSingle());
                        break;

                    case JsonValueKind.Array:
                        var arr = prop.Value;
                        var len = arr.GetArrayLength();
                        w.Write((byte)2); /* vector */
                        w.Write(len);
                        for (int i = 0; i < len; i++)
                            w.Write(arr[i].GetSingle());
                        break;

                    case JsonValueKind.String:
                        w.Write((byte)3); /* string (texture ref) */
                        w.Write(prop.Value.GetString() ?? string.Empty);
                        break;

                    case JsonValueKind.True:
                    case JsonValueKind.False:
                        w.Write((byte)4); /* bool */
                        w.Write(prop.Value.GetBoolean());
                        break;

                    default:
                        w.Write((byte)0); /* unknown */
                        break;
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
