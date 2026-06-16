using Neverness.Runtime.Assets.Registry;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 資產依賴關係追蹤。
///
/// 遷移後：直接調用 C# <see cref="DependencyTable"/>，不再經過 Native ABI。
/// </summary>
public static class DependencyTracking
{
    /// <summary>
    /// 記錄資產依賴（委託給 C# DependencyTable）。
    /// </summary>
    public static void RecordDependency(GUID asset, GUID dependency)
    {
        if (asset.IsZero || dependency.IsZero)
            return;

        AssetRegistry.Instance.DependencyTable.AddDependency(asset, dependency);
    }

    /// <summary>
    /// 取得依賴數量。
    /// </summary>
    public static uint GetDependencyCount(GUID asset)
    {
        if (asset.IsZero) return 0;
        return AssetRegistry.Instance.DependencyTable.GetDependencyCount(asset);
    }

    /// <summary>
    /// 取得指定索引之依賴 GUID。
    /// </summary>
    public static bool TryGetDependencyAt(GUID asset, uint index, out GUID dependency)
    {
        dependency = GUID.Zero;
        if (asset.IsZero) return false;
        return AssetRegistry.Instance.DependencyTable.TryGetDependencyAt(asset, index, out dependency);
    }

    /// <summary>
    /// 取得所有依賴。
    /// </summary>
    public static IReadOnlyList<GUID> GetDependencies(GUID asset)
    {
        if (asset.IsZero) return Array.Empty<GUID>();
        return AssetRegistry.Instance.DependencyTable.GetDependencies(asset);
    }

    /// <summary>
    /// 取得反向依賴數量。
    /// </summary>
    public static uint GetReverseDependencyCount(GUID asset)
    {
        if (asset.IsZero) return 0;
        return AssetRegistry.Instance.DependencyTable.GetReverseDependencyCount(asset);
    }

    /// <summary>
    /// 取得所有反向依賴。
    /// </summary>
    public static IReadOnlyList<GUID> GetReverseDependencies(GUID asset)
    {
        if (asset.IsZero) return Array.Empty<GUID>();
        return AssetRegistry.Instance.DependencyTable.GetReverseDependencies(asset);
    }

    /// <summary>清空（單元測試重置）。</summary>
    internal static void ClearForTesting()
    {
        AssetRegistry.Instance.DependencyTable.Clear();
    }
}
