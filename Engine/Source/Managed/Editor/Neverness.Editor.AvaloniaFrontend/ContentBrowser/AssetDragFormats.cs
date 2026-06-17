using Avalonia.Input;

namespace Neverness.Editor.AvaloniaFrontend.ContentBrowser;

/// <summary>
/// 资产拖拽数据格式常量。
/// 用于 ContentBrowser → Inspector 等面板之间的资产拖拽传输。
/// </summary>
public static class AssetDragFormats
{
    /// <summary>资产文件系统绝对路径（string）。</summary>
    public static readonly DataFormat<string> SystemPath =
        DataFormat.CreateStringApplicationFormat("nnasset-systempath");

    /// <summary>资产虚拟路径（string，如 "Content/Scenes/main.scene"）。</summary>
    public static readonly DataFormat<string> VirtualPath =
        DataFormat.CreateStringApplicationFormat("nnasset-virtualpath");

    /// <summary>
    /// 创建包含资产路径的 DataTransfer。
    /// </summary>
    /// <param name="systemPath">文件系统绝对路径。</param>
    /// <param name="virtualPath">资产虚拟路径（可选）。</param>
    public static DataTransfer CreateTransfer(string systemPath, string? virtualPath = null)
    {
        var transfer = new DataTransfer();
        var item = new DataTransferItem();
        item.Set(SystemPath, systemPath);
        if (!string.IsNullOrEmpty(virtualPath))
            item.Set(VirtualPath, virtualPath);
        transfer.Add(item);
        return transfer;
    }

    /// <summary>
    /// 从 IDataTransfer 中提取资产路径（同步）。
    /// 优先返回虚拟路径，回退到系统路径。
    /// </summary>
    public static string? GetAssetPath(IDataTransfer data)
    {
        // 优先虚拟路径
        if (data.Contains(VirtualPath))
        {
            foreach (var item in data.GetItems(VirtualPath))
            {
                var val = item.TryGetValue(VirtualPath);
                if (!string.IsNullOrEmpty(val))
                    return val;
            }
        }

        // 回退到系统路径
        if (data.Contains(SystemPath))
        {
            foreach (var item in data.GetItems(SystemPath))
            {
                var val = item.TryGetValue(SystemPath);
                if (!string.IsNullOrEmpty(val))
                    return val;
            }
        }

        return null;
    }

    /// <summary>从 DragEventArgs 中提取资产路径。</summary>
    public static string? GetAssetPathFromDrag(DragEventArgs e)
    {
        return GetAssetPath(e.DataTransfer);
    }
}
