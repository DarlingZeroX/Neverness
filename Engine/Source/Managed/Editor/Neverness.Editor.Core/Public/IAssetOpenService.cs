namespace Neverness.Editor.Core.Public;

/// <summary>
/// 资产打开服务接口——通过 Core 服务定位器暴露。
/// Assets 模块实现，Controller 通过服务定位器消费。
/// </summary>
public interface IAssetOpenService
{
    /// <summary>按文件系统路径打开资产。</summary>
    Task<bool> OpenByPathAsync(string filePath);

    /// <summary>按资产 GUID 打开资产。</summary>
    Task<bool> OpenByGuidAsync(string guidHex);

    /// <summary>按资产虚拟路径打开资产（如 asset://path/to/file）。</summary>
    Task<bool> OpenByVirtualPathAsync(string virtualPath);
}
