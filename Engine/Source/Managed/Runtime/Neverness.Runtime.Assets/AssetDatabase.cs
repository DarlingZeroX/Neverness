using Neverness.Runtime.Assets.Registry;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 託管資產登記表。
///
/// 遷移後：直接調用 C# <see cref="AssetRegistry"/>，不再經過 Native ABI。
/// </summary>
public static class AssetDatabase
{
    /// <summary>已登記資產數量。</summary>
    public static int Count => AssetRegistry.Instance.AssetCount;

    /// <summary>
    /// 登記虛擬路徑與 GUID。
    /// </summary>
    public static bool Register(NVirtualPath path, GUID guid)
    {
        if (path.IsEmpty || guid.IsZero)
            return false;

        return AssetRegistry.Instance.RegisterAsset(path.FullPath, guid);
    }

    /// <summary>
    /// 依路徑解析 GUID。
    /// </summary>
    public static bool TryResolveGuid(NVirtualPath path, out GUID guid)
    {
        guid = GUID.Zero;
        if (path.IsEmpty)
            return false;

        return AssetRegistry.Instance.TryResolveGuid(path.FullPath, out guid);
    }

    /// <summary>
    /// 依 GUID 解析虛擬路徑。
    /// </summary>
    public static bool TryResolvePath(GUID guid, out NVirtualPath path)
    {
        path = default;
        if (guid.IsZero)
            return false;

        if (AssetRegistry.Instance.TryResolvePath(guid, out var pathStr) && pathStr != null)
        {
            path = new NVirtualPath(pathStr);
            return true;
        }
        return false;
    }

    /// <summary>清空託管快取（單元測試重置）。</summary>
    public static void ClearForTesting()
    {
        AssetRegistry.Instance.GuidTable.Clear();
    }
}
