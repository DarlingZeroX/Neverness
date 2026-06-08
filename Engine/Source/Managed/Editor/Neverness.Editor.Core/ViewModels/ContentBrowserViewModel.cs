using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// 内容浏览器 ViewModel——暴露 ContentBrowser 的状态供 View 层渲染。
///
/// 设计说明：
/// - ContentBrowser 类本身已是纯数据+操作（无 ImGui），可直接作为 Model 层
/// - ViewModel 包装 ContentBrowser，提供属性变更通知
/// - Controller 处理用户交互，调用 ContentBrowser 的操作方法
/// </summary>
public class ContentBrowserViewModel : ViewModelBase
{
    // ── 当前目录状态 ──
    private string _currentDirectory = "";
    private string _assetDirectory = "";
    private string _searchFilter = "";
    private bool _isRefreshed;

    // ── 选中状态 ──
    private int _selectedItemIndex = -1;

    // ── 视图模式 ──
    private ContentBrowserViewMode _viewMode = ContentBrowserViewMode.Grid;

    // ── 属性 ──

    /// <summary>当前浏览的目录路径。</summary>
    public string CurrentDirectory
    {
        get => _currentDirectory;
        set => SetProperty(ref _currentDirectory, value);
    }

    /// <summary>资产根目录。</summary>
    public string AssetDirectory
    {
        get => _assetDirectory;
        set => SetProperty(ref _assetDirectory, value);
    }

    /// <summary>搜索过滤文本。</summary>
    public string SearchFilter
    {
        get => _searchFilter;
        set => SetProperty(ref _searchFilter, value);
    }

    /// <summary>是否已刷新（需要重绘）。</summary>
    public bool IsRefreshed
    {
        get => _isRefreshed;
        set => SetProperty(ref _isRefreshed, value);
    }

    /// <summary>当前选中的项目索引。</summary>
    public int SelectedItemIndex
    {
        get => _selectedItemIndex;
        set => SetProperty(ref _selectedItemIndex, value);
    }

    /// <summary>视图模式（网格/列表）。</summary>
    public ContentBrowserViewMode ViewMode
    {
        get => _viewMode;
        set => SetProperty(ref _viewMode, value);
    }

    /// <summary>是否可以后退（不在根目录）。</summary>
    public bool CanGoBack => _currentDirectory != _assetDirectory;

    // ── 方法（Controller 调用） ──

    /// <summary>更新当前目录（由 Controller 调用）。</summary>
    public void UpdateCurrentDirectory(string directory, string assetDirectory)
    {
        _currentDirectory = directory;
        _assetDirectory = assetDirectory;
        OnPropertyChanged(nameof(CurrentDirectory));
        OnPropertyChanged(nameof(AssetDirectory));
        OnPropertyChanged(nameof(CanGoBack));
    }

    /// <summary>标记已刷新。</summary>
    public void MarkRefreshed()
    {
        _isRefreshed = true;
        OnPropertyChanged(nameof(IsRefreshed));
    }

    /// <summary>清除刷新标记。</summary>
    public void ClearRefreshedFlag()
    {
        _isRefreshed = false;
        OnPropertyChanged(nameof(IsRefreshed));
    }
}

/// <summary>内容浏览器视图模式。</summary>
public enum ContentBrowserViewMode
{
    Grid,
    List
}
