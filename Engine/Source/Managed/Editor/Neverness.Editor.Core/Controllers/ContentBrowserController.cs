using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Core.Controllers;

/// <summary>
/// 内容浏览器 Controller——处理用户交互。
/// 通过 IContentBrowserService 访问文件系统数据，不直接依赖 ContentBrowser。
/// </summary>
public class ContentBrowserController : IController
{
    private readonly ContentBrowserViewModel _viewModel;
    private readonly IContentBrowserService _contentService;
    private readonly IAssetOpenService? _assetOpenService;
    private readonly Stack<string> _navigationHistory = new();

    public ContentBrowserController(ContentBrowserViewModel viewModel, IContentBrowserService contentService)
    {
        _viewModel = viewModel;
        _contentService = contentService;

        // 尝试获取资产打开服务（可选依赖）
        try
        {
            var context = EditorCoreModule.Context;
            context.TryGetService<IAssetOpenService>(out _assetOpenService);
        }
        catch
        {
            // EditorCoreModule 可能尚未初始化
        }
    }

    public void Initialize()
    {
        // 订阅目录变更事件
        _contentService.DirectoryChanged += OnDirectoryChanged;

        // 初始打开项目根目录
        var rootPath = _contentService.ProjectDirectory;
        if (!string.IsNullOrEmpty(rootPath))
        {
            OpenDirectory(rootPath);
        }
    }

    public void Shutdown()
    {
        _contentService.DirectoryChanged -= OnDirectoryChanged;
    }

    /// <summary>打开目录。</summary>
    public void OpenDirectory(string path)
    {
        // 保存当前目录到历史
        var currentDir = _viewModel.CurrentDirectory;
        if (!string.IsNullOrEmpty(currentDir))
        {
            _navigationHistory.Push(currentDir);
        }

        _contentService.OpenDirectory(path);
        UpdateViewModel(path);
    }

    /// <summary>后退到上一个目录。</summary>
    public void GoBack()
    {
        if (_contentService.CanGoBack && _navigationHistory.Count > 0)
        {
            var prevPath = _navigationHistory.Pop();
            _contentService.OpenDirectory(prevPath);
            UpdateViewModel(prevPath);
        }
    }

    /// <summary>刷新当前目录。</summary>
    public void RefreshDirectory()
    {
        _contentService.Refresh();
        UpdateViewModel(_viewModel.CurrentDirectory);
    }

    /// <summary>创建新文件夹。</summary>
    public void CreateNewFolder()
    {
        // 尝试找到一个不重复的名称
        var folderName = "New Folder";
        var counter = 1;

        // 检查名称是否已存在，如果存在则递增编号
        while (!_contentService.CreateNewFolder(folderName))
        {
            folderName = $"New Folder ({counter++})";

            // 防止无限循环
            if (counter > 100)
            {
                Console.WriteLine("[ContentBrowserController] 无法创建新文件夹：名称冲突");
                return;
            }
        }

        RefreshDirectory();
    }

    /// <summary>删除项目。</summary>
    public void DeleteItem(string path)
    {
        if (_contentService.DeleteItem(path))
        {
            RefreshDirectory();
        }
    }

    /// <summary>重命名项目。</summary>
    public void RenameItem(string oldPath, string newName)
    {
        if (_contentService.RenameItem(oldPath, newName))
        {
            RefreshDirectory();
        }
    }

    /// <summary>设置搜索过滤。</summary>
    public void SetSearchFilter(string filter)
    {
        _viewModel.SearchFilter = filter;
    }

    /// <summary>设置视图模式。</summary>
    public void SetViewMode(ContentBrowserViewMode mode)
    {
        _viewModel.ViewMode = mode;
    }

    /// <summary>选择项目。</summary>
    public void SelectItem(int index)
    {
        _viewModel.SelectedItemIndex = index;
    }

    /// <summary>打开文件（双击）。</summary>
    public async void OpenFile(string systemPath)
    {
        if (_assetOpenService == null)
        {
            Console.WriteLine($"[ContentBrowserController] AssetOpenService 不可用，无法打开: {systemPath}");
            return;
        }

        try
        {
            // 获取资产虚拟路径（与旧代码行为一致）
            var virtualPath = _contentService.GetAssetVirtualPath(systemPath);
            if (!virtualPath.IsEmpty)
            {
                var success = await _assetOpenService.OpenByVirtualPathAsync(virtualPath.ToString());
                if (success) return;
            }

            // 回退：使用文件系统路径
            var fallbackSuccess = await _assetOpenService.OpenByPathAsync(systemPath);
            if (!fallbackSuccess)
            {
                Console.WriteLine($"[ContentBrowserController] 无法打开文件: {systemPath}");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ContentBrowserController] 打开文件异常: {systemPath} → {ex.Message}");
        }
    }

    /// <summary>获取当前目录的文件列表。</summary>
    public IReadOnlyList<ContentFileNode> GetFiles()
    {
        var node = _contentService.GetCurrentDirectoryNode();
        return node.Files;
    }

    /// <summary>获取当前目录的子目录列表。</summary>
    public IReadOnlyList<ContentDirectoryNode> GetSubdirectories()
    {
        var node = _contentService.GetCurrentDirectoryNode();
        return node.Directories;
    }

    /// <summary>获取目录树根节点（用于左侧目录树）。</summary>
    public ContentDirectoryNode GetDirectoryTreeRoot()
    {
        return _contentService.GetDirectoryTreeRoot();
    }

    /// <summary>获取当前目录路径。</summary>
    public string GetCurrentDirectory()
    {
        return _viewModel.CurrentDirectory;
    }

    /// <summary>更新 ViewModel 状态。</summary>
    private void UpdateViewModel(string path)
    {
        _viewModel.UpdateCurrentDirectory(path, _contentService.ProjectDirectory);
        _viewModel.MarkRefreshed();
    }

    /// <summary>目录变更回调。</summary>
    private void OnDirectoryChanged(string newPath)
    {
        UpdateViewModel(newPath);
    }
}
