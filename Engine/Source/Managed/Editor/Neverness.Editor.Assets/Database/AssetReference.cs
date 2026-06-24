using System.Runtime.InteropServices;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产引用（Editor 侧）。
///
/// 在 Inspector 中序列化的资产引用，存储 GUID。
/// 与 Runtime 侧 AssetHandle 配合使用。
///
/// 用法：
///   [SerializeField] private AssetReference _materialRef;
///   // Inspector 中拖入资产 → 自动写入 GUID
///   // 运行时: var handle = _materialRef.LoadAsync();
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct AssetReference
{
    /// <summary>资产 GUID。</summary>
    public GUID Guid;

    /// <summary>可读的资产路径（编辑器显示用，不参与序列化）。</summary>
    [NonSerialized]
    public NVirtualPath? CachedPath;

    public AssetReference(GUID guid)
    {
        Guid = guid;
        CachedPath = null;
    }

    public AssetReference(string guidHex)
    {
        Guid = GUID.Parse(guidHex);
        CachedPath = null;
    }

    /// <summary>是否有效（GUID 非零）。</summary>
    public readonly bool IsValid => !Guid.IsZero;

    /// <summary>获取资产的虚拝路径。</summary>
    public readonly NVirtualPath? GetPath()
    {
        if (Guid.IsZero) return null;
        if (EditorAssetDatabase.TryGetPath(Guid, out var path))
            return path;
        return null;
    }

    /// <summary>加载资产的显示名称。</summary>
    public readonly string GetDisplayName()
    {
        var path = GetPath();
        if (path == null) return Guid.ToUuidString();
        return path.Value.FileNameWithoutExtension;
    }

    /// <inheritdoc />
    public override readonly string ToString()
    {
        return IsValid ? $"AssetReference({GetDisplayName()})" : "AssetReference(Invalid)";
    }
}
