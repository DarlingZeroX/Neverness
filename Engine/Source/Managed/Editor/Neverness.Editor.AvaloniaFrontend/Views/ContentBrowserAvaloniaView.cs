using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 内容浏览器 Avalonia View——Unreal Content Browser 风格。
///
/// 布局：
/// ┌─────────────────────────────────────────────┐
/// │ [◀][▶]  Content/Scenes         [🔍 Search]  │ ← 工具栏
/// ├──────────┬────────────────────────────────────┤
/// │ ▾ Content│ ┌──────┐ ┌──────┐ ┌──────┐       │
/// │   Scenes │ │ SCENE│ │ PREFB│ │ MATER│       │
/// │   Prefabs│ │  🗺️  │ │  🧊  │ │  📋  │       │
/// │          │ │ main │ │player│ │ tree │       │
/// │          │ └──────┘ └──────┘ └──────┘       │
/// ├──────────┴────────────────────────────────────┤
/// │ Content/Scenes                  2 items sel.  │ ← 状态栏
/// └─────────────────────────────────────────────┘
/// </summary>
public class ContentBrowserAvaloniaView : AvaloniaViewBase
{
    /* ======================== ViewModel / Controller ======================== */

    private ContentBrowserViewModel? _viewModel;
    private ContentBrowserController? _controller;

    /* ======================== 子组件 ======================== */

    private ContentBrowserToolbar? _toolbar;
    private ContentBrowserStatusBar? _statusBar;
    private ContentBrowserDirectoryTree? _directoryTree;
    private ContentBrowserThumbnailGrid? _thumbnailGrid;
    private ContentBrowserInteractions? _interactions;

    /* ======================== UI 容器 ======================== */

    private DockPanel? _root;
    private Grid? _mainContent;

    /* ======================== 构造 / 绑定 ======================== */

    public ContentBrowserAvaloniaView() : base("ContentBrowser") { }

    public override Type ViewModelType => typeof(ContentBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ContentBrowserViewModel)viewModel;

        _root = new DockPanel { Background = BgMain };

        // ── 工具栏（顶部）──
        _toolbar = new ContentBrowserToolbar(_viewModel, OnNavigate);
        var toolbarControl = _toolbar.Create();
        DockPanel.SetDock(toolbarControl, Avalonia.Controls.Dock.Top);
        _root.Children.Add(toolbarControl);

        // ── 状态栏（底部）──
        _statusBar = new ContentBrowserStatusBar(_viewModel);
        var statusBarControl = _statusBar.Create();
        DockPanel.SetDock(statusBarControl, Avalonia.Controls.Dock.Bottom);
        _root.Children.Add(statusBarControl);

        // ── 主内容区 ──
        _mainContent = new Grid();
        _mainContent.ColumnDefinitions.Add(new ColumnDefinition(220, GridUnitType.Pixel) { MinWidth = 120 });
        _mainContent.ColumnDefinitions.Add(new ColumnDefinition(4, GridUnitType.Pixel));
        _mainContent.ColumnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));

        // 可拖拽分割线
        var splitter = new GridSplitter
        {
            Width = 4,
            Background = ContentBrowserColors.Separator,
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Stretch,
            Cursor = new Cursor(StandardCursorType.SizeWestEast),
            [Grid.ColumnProperty] = 1,
        };
        _mainContent.Children.Add(splitter);

        _root.Children.Add(_mainContent);

        Content = _root;

        // 如果 controller 已设置，创建子组件
        if (_controller != null)
            CreateChildComponents();
    }

    public override void Unbind()
    {
        _interactions?.Dispose();
        _interactions = null;

        _thumbnailGrid?.Dispose();
        _thumbnailGrid = null;

        _toolbar = null;
        _statusBar = null;
        _directoryTree = null;

        _root = null;
        _mainContent = null;

        _viewModel = null;
        _controller = null;
    }

    public void SetController(ContentBrowserController controller)
    {
        _controller = controller;

        // 如果已经绑定，创建子组件并刷新
        if (_viewModel != null && _mainContent != null)
        {
            CreateChildComponents();
        }
    }

    /* ======================== 子组件创建 ======================== */

    private void CreateChildComponents()
    {
        if (_viewModel == null || _controller == null || _mainContent == null)
            return;

        // 清理旧的子组件
        _interactions?.Dispose();
        _interactions = null;
        _thumbnailGrid?.Dispose();
        _thumbnailGrid = null;

        // 移除旧的目录树和缩略图（保留分割线）
        var splitter = _mainContent.Children.Count > 0 ? _mainContent.Children[0] : null;
        _mainContent.Children.Clear();
        if (splitter != null)
            _mainContent.Children.Add(splitter);

        // 左侧目录树面板
        _directoryTree = new ContentBrowserDirectoryTree(_controller, OnDirectorySelected);
        var treeControl = _directoryTree.Create();
        _mainContent.Children.Add(treeControl);

        // 右侧缩略图区域
        _thumbnailGrid = new ContentBrowserThumbnailGrid(
            _viewModel, _controller,
            OnOpenDirectory, OnOpenFile, OnSelectionCountChanged);
        var thumbnailControl = _thumbnailGrid.Create();
        _mainContent.Children.Add(thumbnailControl);

        // 交互逻辑
        _interactions = new ContentBrowserInteractions(
            _viewModel, _controller,
            _toolbar!, _statusBar!,
            _directoryTree, _thumbnailGrid);

        // 初始刷新
        _directoryTree.Refresh();
        _toolbar!.RefreshBreadcrumb();
        _thumbnailGrid.Refresh();
    }

    /* ======================== 回调方法 ======================== */

    private void OnNavigate(string command)
    {
        if (command == "back")
            _controller?.GoBack();
        // TODO: forward
    }

    private void OnDirectorySelected(string path)
    {
        _controller?.OpenDirectory(path);
    }

    private void OnOpenDirectory(string path)
    {
        _controller?.OpenDirectory(path);
    }

    private void OnOpenFile(string path)
    {
        _controller?.OpenFile(path);
    }

    private void OnSelectionCountChanged(int count)
    {
        _statusBar?.Update(count);
    }
}
