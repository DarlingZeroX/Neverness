using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Threading;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.AvaloniaFrontend.Inspectors;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 属性检查器 Avalonia View——显示实体组件属性。
///
/// 实现细节：
/// - 显示选中实体的组件列表
/// - 每个组件使用对应的 Inspector 渲染
/// - Add Component 按钮
/// - 绑定到 InspectorViewModel + InspectorController
///
/// 设计约束：
/// - 不做 PropertyGrid（当前组件太少，抽象成本太高）
/// - 直接硬编码每个 Inspector
/// </summary>
public class InspectorAvaloniaView : AvaloniaViewBase
{
    private InspectorViewModel? _viewModel;
    private InspectorController? _controller;
    private StackPanel? _componentPanel;
    private TextBlock? _entityNameLabel;
    private CheckBox? _activeCheckBox;

    public InspectorAvaloniaView() : base("Inspector")
    {
    }

    public override Type ViewModelType => typeof(InspectorViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (InspectorViewModel)viewModel;

        var panel = new DockPanel();

        // ── 实体头部（顶部） ──
        var entityHeader = CreateEntityHeader();
        DockPanel.SetDock(entityHeader, Avalonia.Controls.Dock.Top);
        panel.Children.Add(entityHeader);

        // ── Add Component 按钮（底部） ──
        var bottomBar = CreateBottomBar();
        DockPanel.SetDock(bottomBar, Avalonia.Controls.Dock.Bottom);
        panel.Children.Add(bottomBar);

        // ── 组件列表（填充剩余空间） ──
        var scrollViewer = new ScrollViewer
        {
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
        };

        _componentPanel = new StackPanel
        {
            Spacing = 2,
            Margin = new Avalonia.Thickness(0, 4),
        };
        scrollViewer.Content = _componentPanel;
        panel.Children.Add(scrollViewer);

        Content = panel;

        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _componentPanel = null;
        _entityNameLabel = null;
        _activeCheckBox = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(InspectorController controller)
    {
        _controller = controller;
    }

    /// <summary>创建实体头部。</summary>
    private DockPanel CreateEntityHeader()
    {
        var header = new DockPanel
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Height = 32,
            Margin = new Avalonia.Thickness(0, 0, 0, 4),
        };

        // 激活复选框
        _activeCheckBox = new CheckBox
        {
            IsChecked = _viewModel?.IsActive ?? true,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(4, 0, 8, 0),
        };
        _activeCheckBox.IsCheckedChanged += (_, _) =>
        {
            // TODO: 设置实体激活状态
        };
        Avalonia.Controls.DockPanel.SetDock(_activeCheckBox, Avalonia.Controls.Dock.Left);
        header.Children.Add(_activeCheckBox);

        // 实体名称
        _entityNameLabel = new TextBlock
        {
            Text = _viewModel?.SelectedEntityName ?? "No Selection",
            FontSize = 12,
            FontWeight = FontWeight.Bold,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };
        header.Children.Add(_entityNameLabel);

        return header;
    }

    /// <summary>创建底部栏。</summary>
    private DockPanel CreateBottomBar()
    {
        var bottomBar = new DockPanel
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Height = 32,
            Margin = new Avalonia.Thickness(0, 4, 0, 0),
        };

        // Add Component 按钮
        var addButton = new Button
        {
            Content = "Add Component",
            MinWidth = 120,
            MinHeight = 24,
            Padding = new Avalonia.Thickness(4, 2),
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };
        addButton.Click += (_, _) =>
        {
            // TODO: 显示 Add Component 弹窗
            Console.WriteLine("[Inspector] Add Component clicked");
        };
        bottomBar.Children.Add(addButton);

        return bottomBar;
    }

    /// <summary>ViewModel 属性变更——确保在 Avalonia UI 线程执行。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        if (propertyName == nameof(InspectorViewModel.SelectedEntityName))
        {
            if (_entityNameLabel != null)
                _entityNameLabel.Text = _viewModel?.SelectedEntityName ?? "No Selection";
        }
        else if (propertyName == nameof(InspectorViewModel.IsActive))
        {
            if (_activeCheckBox != null)
                _activeCheckBox.IsChecked = _viewModel?.IsActive ?? true;
        }
        else if (propertyName == nameof(InspectorViewModel.Components))
        {
            // 事件可能从主线程触发，Avalonia 控件只能在 UI 线程修改
            if (Dispatcher.UIThread.CheckAccess())
                RebuildComponentList();
            else
                Dispatcher.UIThread.Invoke(RebuildComponentList);
        }
    }

    /// <summary>重建组件列表。</summary>
    private void RebuildComponentList()
    {
        if (_componentPanel == null || _viewModel == null) return;

        _componentPanel.Children.Clear();

        if (!_viewModel.HasSelection)
        {
            var noSelectionText = new TextBlock
            {
                Text = "No entity selected",
                FontSize = 12,
                Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
                HorizontalAlignment = HorizontalAlignment.Center,
                Margin = new Avalonia.Thickness(0, 20),
            };
            _componentPanel.Children.Add(noSelectionText);
            return;
        }

        // 为每个组件创建 Inspector
        foreach (var component in _viewModel.Components)
        {
            var inspectorUI = AvaloniaComponentInspectorRegistry.CreateInspectorUI(
                (ulong)_viewModel.SelectedEntityId,
                (ulong)_viewModel.SelectedEntityId,
                component.TypeId);

            if (inspectorUI != null)
            {
                _componentPanel.Children.Add(inspectorUI);
            }
        }
    }
}
