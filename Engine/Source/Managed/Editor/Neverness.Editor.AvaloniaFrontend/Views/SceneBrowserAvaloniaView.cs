using Avalonia;
using Avalonia.Controls;
using Avalonia.Threading;
using Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using static Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser.SceneBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 场景浏览器 Avalonia View——显示实体层级树。
///
/// 实现细节：
/// - TreeView 显示实体层级（虚拟化）
/// - 搜索栏过滤
/// - 工具栏：展开/折叠全部
/// - 右键菜单（重命名/复制/删除）
/// - 拖拽重设父节点
/// - 绑定到 SceneBrowserViewModel + SceneBrowserController
/// </summary>
public class SceneBrowserAvaloniaView : AvaloniaViewBase
{
    private SceneBrowserViewModel? _viewModel;
    private SceneBrowserController? _controller;

    // ── 子组件 ──
    private SceneBrowserUIComponents? _uiComponents;
    private SceneBrowserEntityTree? _entityTree;

    public SceneBrowserAvaloniaView() : base("SceneBrowser")
    {
    }

    public override Type ViewModelType => typeof(SceneBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (SceneBrowserViewModel)viewModel;

        // 创建子组件
        _uiComponents = new SceneBrowserUIComponents(_viewModel);
        _entityTree = new SceneBrowserEntityTree(_viewModel);

        // 创建 Avalonia 控件树
        var root = new DockPanel { Background = BgMain };

        // ── 工具栏 ──
        var toolbar = _uiComponents.CreateToolbar(
            onExpandAll: () => _viewModel.ExpandAll(),
            onCollapseAll: () => _viewModel.CollapseAll());
        DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        root.Children.Add(toolbar);

        // ── 状态栏 ──
        var statusBar = _uiComponents.CreateStatusBar();
        DockPanel.SetDock(statusBar, Avalonia.Controls.Dock.Bottom);
        root.Children.Add(statusBar);

        // ── 搜索栏 ──
        var searchBar = _uiComponents.CreateSearchBar();
        DockPanel.SetDock(searchBar, Avalonia.Controls.Dock.Top);
        root.Children.Add(searchBar);

        // ── 分割线 ──
        var separator = new Border
        {
            Height = 1,
            Background = SceneBrowserColors.Separator,
            Margin = new Thickness(0),
        };
        DockPanel.SetDock(separator, Avalonia.Controls.Dock.Top);
        root.Children.Add(separator);

        // ── 实体树 ──
        var entityTree = _entityTree.Create(onSelectionChanged: handle =>
        {
            _viewModel.Select(handle);
            _uiComponents.UpdateStatusBar();
        });
        root.Children.Add(entityTree);

        // 设置为 UserControl 的内容
        Content = root;

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _uiComponents = null;
        _entityTree = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(SceneBrowserController controller)
    {
        _controller = controller;
    }

    /// <summary>ViewModel 属性变更——确保在 Avalonia UI 线程执行。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        if (propertyName == nameof(SceneBrowserViewModel.VisibleNodes))
        {
            // 事件从主线程（SceneWorld.CreateEntity → Emit → RefreshTree）触发，
            // Avalonia 控件只能在 UI 线程修改
            if (Dispatcher.UIThread.CheckAccess())
                RebuildTreeView();
            else
                Dispatcher.UIThread.Invoke(RebuildTreeView);
        }
        else if (propertyName == nameof(SceneBrowserViewModel.SelectedEntityId))
        {
            _uiComponents?.UpdateStatusBar();
        }
    }

    /// <summary>重建 TreeView。</summary>
    private void RebuildTreeView()
    {
        _entityTree?.Rebuild();
        _uiComponents?.UpdateStatusBar();
    }
}
