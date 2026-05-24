using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产打开服务——Editor 层统一入口。
///
/// 流程：
///   1. 通过 <see cref="EditorAssetDatabase"/> 解析 GUID / TypeId
///   2. 通过 <see cref="AssetOpenerRegistry"/> 查找匹配的 Opener
///   3. 构建 <see cref="AssetOpenContext"/> 并执行
///
/// 注册到 <see cref="Neverness.Editor.Core.Public.IEditorContext"/> 后，
/// ContentBrowser / 菜单 / 快捷键均可消费此服务。
/// </summary>
public sealed class AssetOpenService
{
    private readonly AssetOpenerRegistry _registry;

    public AssetOpenService(AssetOpenerRegistry registry)
    {
        _registry = registry;
    }

    /// <summary>按虚拟路径打开资产。</summary>
    public async Task<bool> OpenAsync(NVirtualPath virtualPath)
    {
        if (virtualPath.IsEmpty)
            return false;

        // 1. 解析 GUID
        if (!EditorAssetDatabase.TryGetGuid(virtualPath, out var guid))
        {
            Console.WriteLine($"[AssetOpenService] 资产未注册: {virtualPath}");
            return false;
        }

        return await OpenAsync(guid);
    }

    /// <summary>按 GUID 打开资产。</summary>
    public async Task<bool> OpenAsync(GUID guid)
    {
        if (guid.IsZero)
            return false;

        // 1. 解析虚拟路径
        if (!EditorAssetDatabase.TryGetPath(guid, out var virtualPath) || virtualPath.IsEmpty)
        {
            Console.WriteLine($"[AssetOpenService] GUID 无对应路径: {guid.ToHexString()}");
            return false;
        }

        // 2. 获取类型 ID
        var typeId = EditorAssetDatabase.GetTypeId(guid);

        // 3. 类型 ID 为 0 时，尝试从扩展名推断
        if (typeId == 0)
        {
            typeId = AssetMeta.InferAssetTypeId(virtualPath.Extension);
        }

        // 4. 查找 Opener
        var opener = _registry.GetOpener(typeId);
        if (opener == null)
        {
            Console.WriteLine($"[AssetOpenService] 无 Opener: {virtualPath} (TypeId={typeId})");
            return false;
        }

        // 5. 构建上下文
        var meta = new AssetMeta
        {
            Guid = guid,
            AssetTypeId = typeId,
        };

        var context = new AssetOpenContext
        {
            Meta = meta,
            VirtualPath = virtualPath,
            Guid = guid,
            AssetTypeId = typeId,
        };

        // 6. 执行
        try
        {
            await opener.OpenAsync(context);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AssetOpenService] 打开资产异常: {virtualPath} → {ex.Message}");
            return false;
        }
    }
}
