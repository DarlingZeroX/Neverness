using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 网格导入器。
///
/// 支持格式：.fbx, .obj, .gltf, .glb
///
/// 导入设置：
///   scale           — 浮点数，缩放系数（默认 1.0）
///   calculateNormals   — true | false（默认 true）
///   calculateTangents  — true | false（默认 true）
///   generateColliders  — true | false（默认 false）
///   importMaterials    — true | false（默认 true）
/// </summary>
[AssetImporter(".fbx", ".obj", ".gltf", ".glb")]
public class MeshImporter : ISettingsAwareImporter
{
    public string[] SupportedExtensions => new[] { ".fbx", ".obj", ".gltf", ".glb" };
    public string DisplayName => "Mesh Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.Mesh);

            var sourceData = context.ReadAllBytes();

            /* TODO: 实际应使用 Assimp/FBX SDK 解析网格数据 */
            var meshData = ParseMesh(context.Extension, sourceData);

            if (meshData.Vertices == null || meshData.Vertices.Length == 0)
                return ImportResult.Fail($"无法解析网格: {context.SourceAssetPath}");

            /* 应用缩放 */
            var scale = context.GetSettingFloat("scale", 1.0f);
            if (Math.Abs(scale - 1.0f) > 0.001f)
            {
                for (int i = 0; i < meshData.Vertices.Length; i++)
                    meshData.Vertices[i] *= scale;
            }

            /* 序列化顶点数据 */
            var vertexBlob = SerializeVertices(meshData);
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.VertexBuffer,
                Data = vertexBlob
            });

            /* 序列化索引数据 */
            if (meshData.Indices != null && meshData.Indices.Length > 0)
            {
                var indexBlob = SerializeIndices(meshData);
                result.Blobs.Add(new ImportedBlob
                {
                    BlobType = AssetTypeId.BlobType.IndexBuffer,
                    Data = indexBlob
                });
            }

            /* 如果有材质引用，收集依赖 */
            if (meshData.MaterialReferences != null)
            {
                foreach (var matPath in meshData.MaterialReferences)
                    context.Dependencies.AddByPath(matPath);
            }

            /* 写入 TypeInfo */
            var typeInfo = new NNMeshTypeInfoCSharp
            {
                VertexCount = (uint)meshData.VertexCount,
                IndexCount = (uint)(meshData.Indices?.Length ?? 0),
                SubMeshCount = (uint)meshData.SubMeshCount
            };
            result.TypeInfo = typeInfo.ToBytes();

            result.Dependencies.AddRange(context.Dependencies.Dependencies);

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"网格导入异常: {ex.Message}");
        }
    }

    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["scale"] = "1.0",
        ["calculateNormals"] = "true",
        ["calculateTangents"] = "true",
        ["generateColliders"] = "false",
        ["importMaterials"] = "true"
    };

    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;
        if (settings.TryGetValue("scale", out var scaleStr) && !float.TryParse(scaleStr, out _))
        {
            errorMessage = $"scale 必须是浮点数: {scaleStr}";
            return false;
        }
        return true;
    }

    /* ========== 内部数据结构 ========== */

    private class ParsedMesh
    {
        public float[] Vertices = Array.Empty<float>();  /* xyz per vertex */
        public float[] Normals = Array.Empty<float>();
        public float[] TexCoords = Array.Empty<float>();
        public float[] Tangents = Array.Empty<float>();
        public uint[]? Indices;
        public int VertexCount;
        public int SubMeshCount = 1;
        public string[]? MaterialReferences;
    }

    private ParsedMesh ParseMesh(string extension, byte[] raw)
    {
        /* TODO: 集成 AssimpNet 或其他网格解析库 */
        /* 当前返回空网格 */
        return new ParsedMesh
        {
            Vertices = Array.Empty<float>(),
            VertexCount = 0,
            SubMeshCount = 1
        };
    }

    private byte[] SerializeVertices(ParsedMesh mesh)
    {
        /* 简单序列化：float[] 直接写入 */
        var buf = new byte[mesh.Vertices.Length * 4];
        Buffer.BlockCopy(mesh.Vertices, 0, buf, 0, buf.Length);
        return buf;
    }

    private byte[] SerializeIndices(ParsedMesh mesh)
    {
        if (mesh.Indices == null) return Array.Empty<byte>();
        var buf = new byte[mesh.Indices.Length * 4];
        Buffer.BlockCopy(mesh.Indices, 0, buf, 0, buf.Length);
        return buf;
    }
}

internal struct NNMeshTypeInfoCSharp
{
    public uint VertexCount;
    public uint IndexCount;
    public uint SubMeshCount;
    public uint _pad;

    public byte[] ToBytes()
    {
        var buf = new byte[16];
        BitConverter.GetBytes(VertexCount).CopyTo(buf, 0);
        BitConverter.GetBytes(IndexCount).CopyTo(buf, 4);
        BitConverter.GetBytes(SubMeshCount).CopyTo(buf, 8);
        BitConverter.GetBytes(_pad).CopyTo(buf, 12);
        return buf;
    }
}
