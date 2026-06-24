using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets.Private.Core;

/// <summary>
/// <see cref="IContentBrowserService"/> 的具体实现——包装 <see cref="ContentBrowser"/> 单例，
/// 通过 Core 服务定位器暴露给 Scene / Inspector 等外部模块。
/// </summary>
internal sealed class ContentBrowserService : IContentBrowserService
{
    public ContentBrowserService()
    {
        // 订阅 ContentBrowser 的内容变更事件，转发为 DirectoryChanged
        var cb = ContentBrowser.Instance;
        if (cb != null)
        {
            cb.ContentChanged += OnContentChanged;
        }
    }

    private void OnContentChanged()
    {
        DirectoryChanged?.Invoke(CurrentDirectory);
    }

    private ContentBrowser Browser =>
        ContentBrowser.Instance
        ?? throw new InvalidOperationException("ContentBrowser is not initialized.");

    /// <summary>目录变更事件。</summary>
    public event Action<string>? DirectoryChanged;

    public string CurrentDirectory => Browser.GetCurrentBrowserDirectory();

    public string ProjectDirectory => Browser.GetAssetDirectory();

    /// <summary>是否有历史记录可后退。</summary>
    public bool CanGoBack => !string.IsNullOrEmpty(Browser.GetPrevDirectory());

    public void Refresh()
    {
        Browser.RefreshDirectory();
        DirectoryChanged?.Invoke(CurrentDirectory);
    }

    public void OpenDirectory(string absolutePath)
    {
        Browser.OpenDirectory(absolutePath);
        DirectoryChanged?.Invoke(absolutePath);
    }

    /// <summary>后退到上一个目录。</summary>
    public void GoBack()
    {
        var prevDir = Browser.GetPrevDirectory();
        if (!string.IsNullOrEmpty(prevDir))
        {
            OpenDirectory(prevDir);
        }
    }

    public IReadOnlyList<string> GetSubdirectories()
    {
        var node = Browser.GetCurrentDirectoryNode();
        return node.Directories.Select(d => d.Name).ToList();
    }

    public IReadOnlyList<string> GetFiles()
    {
        var node = Browser.GetCurrentDirectoryNode();
        return node.Files.Select(f => f.Name + f.Extension).ToList();
    }

    /// <summary>获取目录树根节点（用于左侧目录树）。</summary>
    public ContentDirectoryNode GetDirectoryTreeRoot()
    {
        var rootNode = Browser.GetDirectoryTreeRootNode();
        return ConvertToDirectoryNode(rootNode);
    }

    /// <summary>获取当前目录节点（包含子目录和文件）。</summary>
    public ContentDirectoryNode GetCurrentDirectoryNode()
    {
        var currentNode = Browser.GetCurrentDirectoryNode();
        return ConvertToDirectoryNode(currentNode);
    }

    /// <summary>创建新文件夹。</summary>
    public bool CreateNewFolder(string folderName)
    {
        try
        {
            var newPath = System.IO.Path.Combine(CurrentDirectory, folderName);
            if (System.IO.Directory.Exists(newPath))
            {
                return false; // 已存在
            }
            Browser.CreateNewDirectory(CurrentDirectory);
            return true;
        }
        catch
        {
            return false;
        }
    }

    /// <summary>删除文件或目录。</summary>
    public bool DeleteItem(string path)
    {
        try
        {
            // 查找对应的 ContentItem
            var currentNode = Browser.GetCurrentDirectoryNode();

            // 先在文件中查找
            var file = currentNode.Files.FirstOrDefault(f => f.SystemPath.FullPath == path);
            if (file != null)
            {
                Browser.DeleteDirectoryItem(file);
                return true;
            }

            // 再在目录中查找
            var dir = currentNode.Directories.FirstOrDefault(d => d.SystemPath.FullPath == path);
            if (dir != null)
            {
                Browser.DeleteDirectoryItem(dir);
                return true;
            }

            return false;
        }
        catch
        {
            return false;
        }
    }

    /// <summary>重命名文件或目录。</summary>
    public bool RenameItem(string oldPath, string newName)
    {
        try
        {
            var currentNode = Browser.GetCurrentDirectoryNode();

            // 先在文件中查找
            var file = currentNode.Files.FirstOrDefault(f => f.SystemPath.FullPath == oldPath);
            if (file != null)
            {
                Browser.RenameDirectoryItem(file, newName);
                return true;
            }

            // 再在目录中查找
            var dir = currentNode.Directories.FirstOrDefault(d => d.SystemPath.FullPath == oldPath);
            if (dir != null)
            {
                Browser.RenameDirectoryItem(dir, newName);
                return true;
            }

            return false;
        }
        catch
        {
            return false;
        }
    }

    /// <summary>获取文件的资产 GUID。</summary>
    public GUID GetAssetGuid(string path)
    {
        var currentNode = Browser.GetCurrentDirectoryNode();
        var file = currentNode.Files.FirstOrDefault(f => f.SystemPath.FullPath == path);
        return file?.AssetGuid ?? GUID.Zero;
    }

    /// <summary>获取文件的资产类型。</summary>
    public string? GetAssetType(string path)
    {
        var currentNode = Browser.GetCurrentDirectoryNode();
        var file = currentNode.Files.FirstOrDefault(f => f.SystemPath.FullPath == path);
        return file?.AssetType;
    }

    /// <summary>获取文件的资产虚拟路径。</summary>
    public NVirtualPath GetAssetVirtualPath(string systemPath)
    {
        var currentNode = Browser.GetCurrentDirectoryNode();
        var file = currentNode.Files.FirstOrDefault(f => f.SystemPath.FullPath == systemPath);
        return file?.AssetPath ?? default;
    }

    /// <summary>将 ContentDirectory 转换为 ContentDirectoryNode。</summary>
    private static ContentDirectoryNode ConvertToDirectoryNode(ContentDirectory node)
    {
        return new ContentDirectoryNode
        {
            Name = node.Name,
            SystemPath = node.SystemPath,
            AssetPath = node.AssetPath,
            Directories = node.Directories.Select(ConvertToDirectoryNode).ToList(),
            Files = node.Files.Select(f => new ContentFileNode
            {
                Name = f.Name,
                SystemPath = f.SystemPath,
                AssetPath = f.AssetPath,
                Extension = f.Extension,
                AssetType = f.AssetType,
                AssetGuid = f.AssetGuid,
                IconTextureId = f.Icon
            }).ToList()
        };
    }
}
