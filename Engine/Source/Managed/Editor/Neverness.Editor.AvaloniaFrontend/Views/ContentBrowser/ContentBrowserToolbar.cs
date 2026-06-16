using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.ViewModels;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器工具栏——导航按钮 + 搜索框 + 面包屑。
/// </summary>
internal sealed class ContentBrowserToolbar
{
    private readonly ContentBrowserViewModel _viewModel;
    private readonly Action<string>? _onNavigate;

    private StackPanel? _breadcrumbBar;
    private ScrollViewer? _breadcrumbScroll;

    /// <summary>面包屑容器引用，供外部访问。</summary>
    internal StackPanel? BreadcrumbBar => _breadcrumbBar;
    internal ScrollViewer? BreadcrumbScroll => _breadcrumbScroll;

    internal ContentBrowserToolbar(ContentBrowserViewModel viewModel, Action<string>? onNavigate)
    {
        _viewModel = viewModel;
        _onNavigate = onNavigate;
    }

    /// <summary>创建工具栏控件。</summary>
    internal Control Create()
    {
        var toolbar = new DockPanel
        {
            Background = BgToolbar,
            Height = 36,
            Margin = new Thickness(0),
        };

        // 导航按钮
        var navPanel = CreateNavigationButtons();
        DockPanel.SetDock(navPanel, Avalonia.Controls.Dock.Left);
        toolbar.Children.Add(navPanel);

        // 搜索框（右侧）
        var searchBox = CreateSearchBox();
        DockPanel.SetDock(searchBox, Avalonia.Controls.Dock.Right);
        toolbar.Children.Add(searchBox);

        // 面包屑（中间填充）
        _breadcrumbBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 0,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0, 8, 0),
        };
        _breadcrumbScroll = new ScrollViewer
        {
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            Content = _breadcrumbBar,
            VerticalAlignment = VerticalAlignment.Center,
        };
        toolbar.Children.Add(_breadcrumbScroll);

        return toolbar;
    }

    /// <summary>刷新面包屑导航。</summary>
    internal void RefreshBreadcrumb()
    {
        if (_breadcrumbBar == null) return;

        _breadcrumbBar.Children.Clear();

        var currentDir = _viewModel.CurrentDirectory;
        var assetDir = _viewModel.AssetDirectory;

        if (string.IsNullOrEmpty(currentDir) || string.IsNullOrEmpty(assetDir))
            return;

        // 根目录
        AddBreadcrumbSegment("Content", assetDir, isFirst: true);

        var relativePath = System.IO.Path.GetRelativePath(assetDir, currentDir);
        if (relativePath == ".") return;

        var segments = relativePath.Split(System.IO.Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);
        var accumulatedPath = assetDir;

        foreach (var segment in segments)
        {
            accumulatedPath = System.IO.Path.Combine(accumulatedPath, segment);
            AddBreadcrumbSegment(segment, accumulatedPath, isFirst: false);
        }
    }

    /* ======================== 私有方法 ======================== */

    private StackPanel CreateNavigationButtons()
    {
        var navPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 1,
            Margin = new Thickness(4, 4, 8, 4),
            VerticalAlignment = VerticalAlignment.Center,
        };

        var backBtn = CreateToolButton("◀", 28);
        backBtn.Click += (_, _) => _onNavigate?.Invoke("back");
        navPanel.Children.Add(backBtn);

        var fwdBtn = CreateToolButton("▶", 28);
        fwdBtn.Click += (_, _) => _onNavigate?.Invoke("forward");
        navPanel.Children.Add(fwdBtn);

        return navPanel;
    }

    private TextBox CreateSearchBox()
    {
        var searchBox = new TextBox
        {
            PlaceholderText = "🔍  Search...",
            Background = BgInput,
            Foreground = TextPrimary,
            BorderBrush = ContentBrowserColors.Separator,
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(3),
            MinWidth = 180,
            MaxWidth = 260,
            Height = 24,
            Padding = new Thickness(8, 4),
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center,
            Margin = new Thickness(0, 0, 8, 0),
        };
        searchBox.TextChanged += (_, _) =>
        {
            _viewModel.SearchFilter = searchBox.Text ?? "";
        };
        return searchBox;
    }

    private void AddBreadcrumbSegment(string label, string path, bool isFirst)
    {
        if (!isFirst)
        {
            _breadcrumbBar!.Children.Add(new TextBlock
            {
                Text = "›",
                Foreground = TextSecondary,
                FontSize = 12,
                VerticalAlignment = VerticalAlignment.Center,
                Margin = new Thickness(4, 0),
            });
        }

        var btn = new Button
        {
            Content = label,
            Padding = new Thickness(4, 1),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Foreground = TextPrimary,
            FontSize = 12,
            CornerRadius = new CornerRadius(2),
        };
        var capturedPath = path;
        btn.Click += (_, _) => _onNavigate?.Invoke(capturedPath);
        btn.PointerEntered += (_, _) => btn.Foreground = TextBright;
        btn.PointerExited += (_, _) => btn.Foreground = TextPrimary;
        _breadcrumbBar!.Children.Add(btn);
    }

    private static Button CreateToolButton(string text, double size)
    {
        var btn = new Button
        {
            Content = text,
            Width = size,
            Height = size,
            Padding = new Thickness(0),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Foreground = TextSecondary,
            FontSize = 12,
            CornerRadius = new CornerRadius(3),
            HorizontalContentAlignment = HorizontalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center,
        };
        btn.PointerEntered += (_, _) => btn.Background = new SolidColorBrush(Color.Parse("#FF4A4A4A"));
        btn.PointerExited += (_, _) => btn.Background = Brushes.Transparent;
        return btn;
    }
}
