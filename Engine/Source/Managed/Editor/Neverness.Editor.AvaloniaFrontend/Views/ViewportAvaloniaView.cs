using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 编辑器视口 Avalonia View——显示 3D 场景。
///
/// 实现细节：
/// - NativeControlHost 嵌入原生渲染窗口
/// - Diligent 直接渲染到原生窗口句柄
/// - 工具栏：聚焦、网格、Gizmo 模式
/// - 绑定到 EditorViewportViewModel + EditorViewportController
///
/// 设计原则：
/// - NativeWindowHandle 不进 ViewModel
/// - 通过 ViewportHostService 管理原生窗口句柄
/// - IViewportSurface 抽象平台差异
/// </summary>
public class ViewportAvaloniaView : AvaloniaViewBase
{
    private EditorViewportViewModel? _viewModel;
    private EditorViewportController? _controller;
    private Panel? _viewportPanel;

    public ViewportAvaloniaView() : base("Viewport")
    {
    }

    public override Type ViewModelType => typeof(EditorViewportViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (EditorViewportViewModel)viewModel;

        // 创建 Avalonia 控件树
        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        Avalonia.Controls.DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 视口区域（暂用占位面板，NativeControlHost 需要应用 manifest） ──
        _viewportPanel = new Panel
        {
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            ClipToBounds = true,
        };

        // TODO: NativeControlHost 需要应用 manifest 支持，暂时用占位文本
        _viewportPanel.Children.Add(new TextBlock
        {
            Text = "Viewport (NativeControlHost 待配置)",
            Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
            FontSize = 16,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        });

        panel.Children.Add(_viewportPanel);

        Content = panel;

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _viewportPanel = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(EditorViewportController controller)
    {
        _controller = controller;
    }

    /// <summary>创建工具栏。</summary>
    private StackPanel CreateToolbar()
    {
        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4),
        };

        // 聚焦按钮
        var focusButton = new Button
        {
            Content = "Focus",
            MinWidth = 60,
            MinHeight = 24,
            Padding = new Avalonia.Thickness(4, 2),
        };
        focusButton.Click += (_, _) =>
        {
            // TODO: 调用 Controller 聚焦选中实体
            Console.WriteLine("[Viewport] Focus clicked");
        };
        toolbar.Children.Add(focusButton);

        // 网格切换
        var gridCheckBox = new CheckBox
        {
            Content = "Grid",
            IsChecked = true,
            VerticalAlignment = VerticalAlignment.Center,
        };
        gridCheckBox.IsCheckedChanged += (_, _) =>
        {
            // TODO: 切换网格显示
            Console.WriteLine($"[Viewport] Grid: {gridCheckBox.IsChecked}");
        };
        toolbar.Children.Add(gridCheckBox);

        // Gizmo 模式
        var gizmoCombo = new ComboBox
        {
            MinWidth = 100,
            MinHeight = 24,
            ItemsSource = new[] { "Translate", "Rotate", "Scale" },
            SelectedIndex = 0,
        };
        gizmoCombo.SelectionChanged += (_, _) =>
        {
            // TODO: 切换 Gizmo 模式
            Console.WriteLine($"[Viewport] Gizmo mode: {gizmoCombo.SelectedIndex}");
        };
        toolbar.Children.Add(gizmoCombo);

        // 视口尺寸信息
        var sizeLabel = new TextBlock
        {
            Text = $"{_viewModel?.ViewportWidth ?? 0}x{_viewModel?.ViewportHeight ?? 0}",
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
            Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
        };
        toolbar.Children.Add(sizeLabel);

        return toolbar;
    }

    /// <summary>ViewModel 属性变更。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        // TODO: 响应 ViewModel 属性变更
    }
}
