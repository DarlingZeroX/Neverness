using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.ViewModels;
using static Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser.SceneBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser;

/// <summary>
/// 场景浏览器 UI 组件——工具栏、搜索栏、状态栏。
/// </summary>
internal sealed class SceneBrowserUIComponents
{
    private readonly SceneBrowserViewModel _viewModel;

    private TextBlock? _statusText;
    private TextBlock? _selectionStatus;
    private TextBox? _searchBox;

    /// <summary>状态文本引用。</summary>
    internal TextBlock? StatusText => _statusText;
    internal TextBlock? SelectionStatus => _selectionStatus;
    internal TextBox? SearchBox => _searchBox;

    internal SceneBrowserUIComponents(SceneBrowserViewModel viewModel)
    {
        _viewModel = viewModel;
    }

    /// <summary>创建工具栏。</summary>
    internal Control CreateToolbar(Action? onExpandAll, Action? onCollapseAll)
    {
        var toolbar = new DockPanel
        {
            Background = BgToolbar,
            Height = 32,
            Margin = new Thickness(0),
        };

        // 左侧按钮组
        var buttonPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
            Margin = new Thickness(4),
            VerticalAlignment = VerticalAlignment.Center,
        };

        // 展开全部按钮
        var expandButton = CreateToolButton("▶", "Expand All");
        expandButton.Click += (_, _) => onExpandAll?.Invoke();
        buttonPanel.Children.Add(expandButton);

        // 折叠全部按钮
        var collapseButton = CreateToolButton("◀", "Collapse All");
        collapseButton.Click += (_, _) => onCollapseAll?.Invoke();
        buttonPanel.Children.Add(collapseButton);

        DockPanel.SetDock(buttonPanel, Avalonia.Controls.Dock.Left);
        toolbar.Children.Add(buttonPanel);

        // 右侧节点计数
        _statusText = new TextBlock
        {
            Text = $"Entities: {_viewModel.NodeCount}",
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0),
            Foreground = TextSecondary,
            FontSize = 11,
        };
        DockPanel.SetDock(_statusText, Avalonia.Controls.Dock.Right);
        toolbar.Children.Add(_statusText);

        return toolbar;
    }

    /// <summary>创建搜索栏。</summary>
    internal Control CreateSearchBar()
    {
        var searchBar = new DockPanel
        {
            Margin = new Thickness(8, 6),
        };

        _searchBox = new TextBox
        {
            PlaceholderText = "🔍  Search entities...",
            Background = BgInput,
            Foreground = TextPrimary,
            BorderBrush = SceneBrowserColors.Separator,
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(3),
            MinHeight = 24,
            Padding = new Thickness(8, 4),
            FontSize = 12,
            VerticalContentAlignment = VerticalAlignment.Center,
        };
        _searchBox.TextChanged += (_, _) =>
        {
            _viewModel.SearchText = _searchBox.Text ?? "";
        };
        searchBar.Children.Add(_searchBox);

        return searchBar;
    }

    /// <summary>创建状态栏。</summary>
    internal Control CreateStatusBar()
    {
        var bar = new DockPanel
        {
            Background = BgStatusBar,
            Height = 24,
            Margin = new Thickness(0),
        };

        // 左侧：选中信息
        _selectionStatus = new TextBlock
        {
            Text = "No selection",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0, 0, 0),
        };
        DockPanel.SetDock(_selectionStatus, Avalonia.Controls.Dock.Left);
        bar.Children.Add(_selectionStatus);

        // 右侧：实体计数
        var countLabel = new TextBlock
        {
            Text = $"Nodes: {_viewModel.NodeCount}",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(0, 0, 8, 0),
            HorizontalAlignment = HorizontalAlignment.Right,
        };
        DockPanel.SetDock(countLabel, Avalonia.Controls.Dock.Right);
        bar.Children.Add(countLabel);

        return bar;
    }

    /// <summary>更新状态栏。</summary>
    internal void UpdateStatusBar()
    {
        if (_statusText != null)
        {
            _statusText.Text = $"Entities: {_viewModel.NodeCount}";
        }

        if (_selectionStatus != null)
        {
            var selectedId = _viewModel.SelectedEntityId;
            _selectionStatus.Text = selectedId != 0 ? $"Selected: {selectedId}" : "No selection";
        }
    }

    /* ======================== 私有方法 ======================== */

    private static Button CreateToolButton(string content, string tooltip)
    {
        var btn = new Button
        {
            Content = content,
            Width = 28,
            Height = 24,
            Padding = new Thickness(0),
            Background = ButtonBg,
            BorderThickness = new Thickness(0),
            Foreground = TextPrimary,
            FontSize = 12,
            CornerRadius = new CornerRadius(3),
            HorizontalContentAlignment = HorizontalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center,
        };
        ToolTip.SetTip(btn, tooltip);
        btn.PointerEntered += (_, _) => btn.Background = ButtonHover;
        btn.PointerExited += (_, _) => btn.Background = ButtonBg;
        return btn;
    }
}
