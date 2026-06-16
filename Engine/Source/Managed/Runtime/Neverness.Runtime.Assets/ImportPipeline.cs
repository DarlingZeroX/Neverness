using Neverness.Runtime.Assets.Registry;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 资产导入管线（Runtime 版本）。
///
/// 迁移后：直接使用 C# <see cref="AssetRegistry.ImportAsset"/> 合成 GUID，
/// 不再经过 EngineNativeApiBootstrap / C++ ABI。
///
/// 注意：Editor 版本的 ImportPipeline（<c>Neverness.Editor.Assets.ImportPipeline</c>）
/// 是独立的完整 8 阶段导入管线，不受此文件影响。
/// </summary>
public static class ImportPipeline
{
    /// <summary>
    /// 导入指定虚拟路径之资产。
    /// 使用 C# AssetRegistry 的 FNV-1a 合成稳定 GUID，并登记映射。
    /// </summary>
    /// <param name="path">虚拟路径。</param>
    /// <returns>已登记之 GUID。</returns>
    public static GUID Import(NVirtualPath path)
    {
        if (path.IsEmpty)
            return GUID.Zero;

        return AssetRegistry.Instance.ImportAsset(path.FullPath);
    }

    /// <summary>
    /// 导入资产（带结果类型）。
    /// 供需要 Success/ErrorMessage 的调用方使用。
    /// </summary>
    /// <param name="path">虚拟路径。</param>
    /// <returns>导入结果。</returns>
    public static ImportAssetResult ImportWithResult(NVirtualPath path)
    {
        if (path.IsEmpty)
            return ImportAssetResult.Fail("路径为空");

        var guid = AssetRegistry.Instance.ImportAsset(path.FullPath);
        if (guid.IsZero)
            return ImportAssetResult.Fail("GUID 合成失败");

        return ImportAssetResult.Ok(guid);
    }
}

/// <summary>
/// Runtime 资产导入结果。
/// </summary>
public sealed class ImportAssetResult
{
    /// <summary>是否成功。</summary>
    public bool Success { get; init; }

    /// <summary>资产 GUID。</summary>
    public GUID AssetGuid { get; init; }

    /// <summary>错误信息（Success=false 时）。</summary>
    public string? ErrorMessage { get; init; }

    public static ImportAssetResult Ok(GUID guid) => new()
    {
        Success = true,
        AssetGuid = guid
    };

    public static ImportAssetResult Fail(string message) => new()
    {
        Success = false,
        ErrorMessage = message
    };
}
