using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets.Private.Core;

/// <summary>
/// 资产打开服务实现——封装 AssetOpenService。
/// 对外暴露 IAssetOpenService 接口，Controller 不直接依赖 Assets 模块。
/// </summary>
internal sealed class AssetOpenServiceImpl : IAssetOpenService
{
    private readonly AssetOpenService _openService;

    public AssetOpenServiceImpl(AssetOpenService openService)
    {
        _openService = openService;
    }

    /// <summary>按文件路径打开资产。</summary>
    public async Task<bool> OpenByPathAsync(string filePath)
    {
        if (string.IsNullOrEmpty(filePath))
            return false;

        // 将文件路径转换为虚拟路径
        var virtualPath = ProjectPaths.GetResourcePath(new NPath(filePath));
        if (virtualPath == null || virtualPath.Value.IsEmpty)
        {
            Console.WriteLine($"[AssetOpenServiceImpl] 无法转换路径: {filePath}");
            return false;
        }

        return await _openService.OpenAsync(virtualPath.Value);
    }

    /// <summary>按资产 GUID 打开资产。</summary>
    public async Task<bool> OpenByGuidAsync(string guidHex)
    {
        if (string.IsNullOrEmpty(guidHex))
            return false;

        var guid = GUID.Parse(guidHex);
        if (guid.IsZero)
            return false;

        return await _openService.OpenAsync(guid);
    }

    /// <summary>按资产虚拟路径打开资产。</summary>
    public async Task<bool> OpenByVirtualPathAsync(string virtualPath)
    {
        if (string.IsNullOrEmpty(virtualPath))
            return false;

        var path = new NVirtualPath(virtualPath);
        if (path.IsEmpty)
            return false;

        return await _openService.OpenAsync(path);
    }
}
