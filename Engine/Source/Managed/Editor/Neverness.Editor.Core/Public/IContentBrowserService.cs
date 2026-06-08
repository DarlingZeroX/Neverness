using Neverness.Runtime.Assets;

namespace Neverness.Editor.Core.Public;

/// <summary>
/// ContentBrowser 服务接口——通过 Core 服务定位器暴露。
/// Scene / Inspector 等模块可消费此接口，无需直接引用 Assets 程序集。
/// </summary>
public interface IContentBrowserService
{
    /// <summary>当前浏览目录的绝对路径。</summary>
    string CurrentDirectory { get; }

    /// <summary>项目资产根目录的绝对路径。</summary>
    string ProjectDirectory { get; }

    /// <summary>目录变更事件。</summary>
    event Action<string>? DirectoryChanged;

    /// <summary>刷新当前目录。</summary>
    void Refresh();

    /// <summary>打开指定目录。</summary>
    void OpenDirectory(string absolutePath);

    /// <summary>后退到上一个目录。</summary>
    void GoBack();

    /// <summary>是否有历史记录可后退。</summary>
    bool CanGoBack { get; }

    /// <summary>当前目录下的所有子目录名。</summary>
    IReadOnlyList<string> GetSubdirectories();

    /// <summary>当前目录下的所有文件名（含扩展名）。</summary>
    IReadOnlyList<string> GetFiles();

    /// <summary>获取目录树根节点（用于左侧目录树）。</summary>
    ContentDirectoryNode GetDirectoryTreeRoot();

    /// <summary>获取当前目录节点（包含子目录和文件）。</summary>
    ContentDirectoryNode GetCurrentDirectoryNode();

    /// <summary>创建新文件夹。</summary>
    bool CreateNewFolder(string folderName);

    /// <summary>删除文件或目录。</summary>
    bool DeleteItem(string path);

    /// <summary>重命名文件或目录。</summary>
    bool RenameItem(string oldPath, string newName);

    /// <summary>获取文件的资产 GUID。</summary>
    GUID GetAssetGuid(string path);

    /// <summary>获取文件的资产类型。</summary>
    string? GetAssetType(string path);

    /// <summary>获取文件的资产虚拟路径。</summary>
    NVirtualPath GetAssetVirtualPath(string systemPath);
}

/// <summary>
/// 目录节点数据——与 ContentBrowser.ContentDirectory 对应。
/// </summary>
public class ContentDirectoryNode
{
    /// <summary>目录名称。</summary>
    public string Name { get; init; } = "";

    /// <summary>文件系统路径。</summary>
    public NPath SystemPath { get; init; }

    /// <summary>资产虚拟路径。</summary>
    public NVirtualPath AssetPath { get; init; }

    /// <summary>子目录列表。</summary>
    public List<ContentDirectoryNode> Directories { get; init; } = new();

    /// <summary>文件列表。</summary>
    public List<ContentFileNode> Files { get; init; } = new();
}

/// <summary>
/// 文件节点数据——与 ContentBrowser.ContentFile 对应。
/// </summary>
public class ContentFileNode
{
    /// <summary>文件名称（不含扩展名）。</summary>
    public string Name { get; init; } = "";

    /// <summary>文件系统路径。</summary>
    public NPath SystemPath { get; init; }

    /// <summary>资产虚拟路径。</summary>
    public NVirtualPath AssetPath { get; init; }

    /// <summary>文件扩展名。</summary>
    public string Extension { get; init; } = "";

    /// <summary>资产类型。</summary>
    public string AssetType { get; init; } = "Unknown";

    /// <summary>资产 GUID。</summary>
    public GUID AssetGuid { get; init; }

    /// <summary>图标纹理 ID。</summary>
    public ulong IconTextureId { get; init; }
}
