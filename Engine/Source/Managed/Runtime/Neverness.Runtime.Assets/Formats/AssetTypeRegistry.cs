namespace Neverness.Runtime.Assets.Formats;

/// <summary>
/// 預定義資產類型 ID 常量（與 C++ NN_TYPE_ID_* 一致）。
/// </summary>
public static class TypeId
{
    public const ulong Texture2D = 0x00000001;
    public const ulong Mesh = 0x00000002;
    public const ulong AudioClip = 0x00000003;
    public const ulong Material = 0x00000004;
    public const ulong Shader = 0x00000005;
    public const ulong Scene = 0x00000006;
    public const ulong Prefab = 0x00000007;
    public const ulong Animation = 0x00000008;
    public const ulong LuaScript = 0x00000009;
    public const ulong VideoClip = 0x0000000A;
    public const ulong CSharpScript = 0x0000000B;
    public const ulong HtmlDocument = 0x0000000C;
}

/// <summary>
/// 預定義 Blob 類型常量（與 C++ NN_BLOB_TYPE_* 一致）。
/// </summary>
public static class BlobType
{
    public const uint Data = 0;
    public const uint MipLevel = 1;
    public const uint VertexBuffer = 2;
    public const uint IndexBuffer = 3;
    public const uint Thumbnail = 4;
    public const uint AudioPcm = 5;
    public const uint AudioSeek = 6;
    public const uint EntityHierarchy = 7;
    public const uint ComponentData = 8;
    public const uint TypeInfo = 9;
    public const uint VideoFrame = 10;
    public const uint VideoSeek = 11;
    public const uint Subtitle = 12;
}

/// <summary>
/// FNV-1a 64-bit 雜湊函數。
/// 與 C++ NNFnv1a64 結果一致。
/// </summary>
public static class Fnv1a64
{
    private const ulong OffsetBasis = 14695981039346656037UL;
    private const ulong Prime = 1099511628211UL;

    /// <summary>
    /// 計算 UTF-8 位元組序列的 FNV-1a 64-bit 雜湊。
    /// </summary>
    public static ulong Hash(ReadOnlySpan<byte> data)
    {
        ulong hash = OffsetBasis;
        for (int i = 0; i < data.Length; i++)
        {
            hash ^= data[i];
            hash *= Prime;
        }
        return hash;
    }

    /// <summary>
    /// 計算字串的 FNV-1a 64-bit 雜湊（UTF-8 編碼）。
    /// </summary>
    public static ulong Hash(string text)
    {
        var byteCount = System.Text.Encoding.UTF8.GetByteCount(text);
        var buffer = byteCount <= 256
            ? stackalloc byte[byteCount]
            : new byte[byteCount];
        System.Text.Encoding.UTF8.GetBytes(text, buffer);
        return Hash(buffer);
    }
}

/// <summary>
/// 資產型別註冊表。
///
/// 使用 FNV-1a 64-bit 雜湊將型別名稱字串映射為 ulong typeId。
/// 同時維護 typeId → 型別名稱的反向映射（除錯/序列化用）。
/// 與 C++ NNAssetTypeRegistry 對應。
/// </summary>
public sealed class AssetTypeRegistry
{
    private readonly Dictionary<string, ulong> _nameToId = new();
    private readonly Dictionary<ulong, string> _idToName = new();
    private readonly object _lock = new();

    /// <summary>全域單例。</summary>
    public static AssetTypeRegistry Instance { get; } = new();

    private AssetTypeRegistry()
    {
        // 註冊 9 個預設型別（與 C++ 初始化一致）
        RegisterType("Texture2D", TypeId.Texture2D);
        RegisterType("Mesh", TypeId.Mesh);
        RegisterType("AudioClip", TypeId.AudioClip);
        RegisterType("Material", TypeId.Material);
        RegisterType("Shader", TypeId.Shader);
        RegisterType("Scene", TypeId.Scene);
        RegisterType("Prefab", TypeId.Prefab);
        RegisterType("Animation", TypeId.Animation);
        RegisterType("LuaScript", TypeId.LuaScript);
        RegisterType("VideoClip", TypeId.VideoClip);
        RegisterType("CSharpScript", TypeId.CSharpScript);
        RegisterType("HtmlDocument", TypeId.HtmlDocument);
    }

    /// <summary>
    /// 註冊型別（名稱 → FNV-1a → typeId）。
    /// 若 predefinedId 非零則使用預定義 ID，否則使用 FNV-1a 雜湊。
    /// </summary>
    public void RegisterType(string typeName, ulong predefinedId = 0)
    {
        lock (_lock)
        {
            var id = predefinedId != 0 ? predefinedId : Fnv1a64.Hash(typeName);
            _nameToId[typeName] = id;
            _idToName[id] = typeName;
        }
    }

    /// <summary>
    /// 依名稱取得 typeId。回傳 0 表示未註冊。
    /// </summary>
    public ulong GetTypeId(string typeName)
    {
        lock (_lock)
        {
            return _nameToId.TryGetValue(typeName, out var id) ? id : 0;
        }
    }

    /// <summary>
    /// 依 typeId 取得名稱。回傳空字串表示未知。
    /// </summary>
    public string GetTypeName(ulong typeId)
    {
        lock (_lock)
        {
            return _idToName.TryGetValue(typeId, out var name) ? name : string.Empty;
        }
    }

    /// <summary>
    /// 型別是否已註冊。
    /// </summary>
    public bool IsRegistered(ulong typeId)
    {
        lock (_lock)
        {
            return _idToName.ContainsKey(typeId);
        }
    }
}
