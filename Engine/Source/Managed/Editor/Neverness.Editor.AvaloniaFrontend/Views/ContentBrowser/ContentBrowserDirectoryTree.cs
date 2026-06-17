using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Public;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器目录树——左侧文件夹树形结构。
/// </summary>
internal sealed class ContentBrowserDirectoryTree
{
    private readonly ContentBrowserController _controller;
    private readonly Action<string>? _onDirectorySelected;

    private TreeView? _directoryTree;

    /// <summary>目录树控件引用。</summary>
    internal TreeView? DirectoryTree => _directoryTree;

    internal ContentBrowserDirectoryTree(ContentBrowserController controller, Action<string>? onDirectorySelected)
    {
        _controller = controller;
        _onDirectorySelected = onDirectorySelected;
    }

    /// <summary>创建目录树面板（含右侧内阴影）。</summary>
    internal Control Create()
    {
        var treePanel = new DockPanel { Background = BgPanel };

        // 四边内阴影（右侧 + 顶部 + 底部，每层一个 Border）
        var treeShadowRight = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x40, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = -3,
                OffsetY = 0,
            }),
        };
        var treeShadowTop = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x30, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = 0,
                OffsetY = 3,
            }),
        };
        var treeShadowBottom = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x30, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = 0,
                OffsetY = -3,
            }),
        };

        var treeShadow = new Grid { [Grid.ColumnProperty] = 0, ClipToBounds = true };
        treeShadow.Children.Add(treePanel);
        treeShadow.Children.Add(treeShadowRight);
        treeShadow.Children.Add(treeShadowTop);
        treeShadow.Children.Add(treeShadowBottom);

        // 标题
        var treeHeader = new TextBlock
        {
            Text = "Content",
            FontSize = 11,
            FontWeight = FontWeight.Bold,
            Foreground = TextSecondary,
            Margin = new Thickness(12, 8, 12, 6),
        };
        DockPanel.SetDock(treeHeader, Avalonia.Controls.Dock.Top);
        treePanel.Children.Add(treeHeader);

        // 目录树
        _directoryTree = new TreeView
        {
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Padding = new Thickness(4, 0, 4, 4),
        };
        _directoryTree.SelectionChanged += OnSelectionChanged;
        treePanel.Children.Add(_directoryTree);

        return treeShadow;
    }

    /// <summary>刷新目录树数据。</summary>
    internal void Refresh()
    {
        if (_directoryTree == null) return;

        _directoryTree.Items.Clear();

        try
        {
            var root = _controller.GetDirectoryTreeRoot();
            if (root == null) return;

            var rootItem = new TreeViewItem
            {
                Header = CreateTreeHeader("Content", isRoot: true),
                IsExpanded = true,
                Tag = root.SystemPath.FullPath,
            };

            foreach (var child in root.Directories)
                rootItem.Items.Add(CreateTreeNode(child));

            _directoryTree.Items.Add(rootItem);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ContentBrowser] 目录树加载失败: {ex.Message}");
        }
    }

    /// <summary>高亮指定路径的目录树节点。</summary>
    internal void HighlightNode(string? targetPath)
    {
        if (_directoryTree == null || string.IsNullOrEmpty(targetPath)) return;
        FindAndSelectTreeNode(_directoryTree.Items, targetPath);
    }

    /* ======================== 私有方法 ======================== */

    private void OnSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (e.AddedItems.Count == 0) return;
        if (e.AddedItems[0] is TreeViewItem item && item.Tag is string path)
            _onDirectorySelected?.Invoke(path);
    }

    private TreeViewItem CreateTreeNode(ContentDirectoryNode node)
    {
        var item = new TreeViewItem
        {
            Header = CreateTreeHeader(node.Name, isRoot: false),
            Tag = node.SystemPath.FullPath,
        };

        foreach (var child in node.Directories)
            item.Items.Add(CreateTreeNode(child));

        return item;
    }

    private static StackPanel CreateTreeHeader(string name, bool isRoot)
    {
        var header = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 6,
            Margin = new Thickness(0, 1),
        };

        header.Children.Add(new TextBlock
        {
            Text = isRoot ? "▾" : "▸",
            FontSize = 10,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Width = 12,
            TextAlignment = TextAlignment.Center,
        });

        header.Children.Add(new TextBlock
        {
            Text = "📁",
            FontSize = 13,
            VerticalAlignment = VerticalAlignment.Center,
        });

        header.Children.Add(new TextBlock
        {
            Text = name,
            FontSize = 12,
            Foreground = isRoot ? TextBright : TextPrimary,
            FontWeight = isRoot ? FontWeight.Bold : FontWeight.Normal,
            VerticalAlignment = VerticalAlignment.Center,
        });

        return header;
    }

    private static bool FindAndSelectTreeNode(System.Collections.IList items, string targetPath)
    {
        foreach (var obj in items)
        {
            if (obj is not TreeViewItem item) continue;

            if (item.Tag is string path && path == targetPath)
            {
                item.IsSelected = true;
                item.BringIntoView();
                return true;
            }

            if (FindAndSelectTreeNode(item.Items, targetPath))
            {
                item.IsExpanded = true;
                return true;
            }
        }
        return false;
    }
}
