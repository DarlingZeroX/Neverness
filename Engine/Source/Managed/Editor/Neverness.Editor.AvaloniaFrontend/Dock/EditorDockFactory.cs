using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.VisualTree;
using Dock.Avalonia.Controls;
using Dock.Model;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.Mvvm;
using Dock.Model.Mvvm.Controls;
using Dock.Model.Mvvm.Core;
using Neverness.Editor.AvaloniaFrontend.Public;

namespace Neverness.Editor.AvaloniaFrontend.Dock;

/// <summary>
/// 编辑器 Dock 工厂——创建默认编辑器布局。
///
/// 参考 Dock 官方 DockCodeOnlyMvvmSample。
///
/// 默认布局（UE 风格）：
/// ┌────────────────────────┬──────────────┐
/// │                        │ Scene Browser│
/// │       Viewport         ├──────────────┤
/// │                        │              │
/// ├──────────┬─────────────┤  Inspector   │
/// │ Content  │  Console    │              │
/// └──────────┴─────────────┴──────────────┘
/// </summary>
public class EditorDockFactory : Factory
{
    public EditorDockFactory()
    {
        // 注册浮动窗口定位器——FloatDockable 需要此定位器创建 IHostWindow 模型
        DefaultHostWindowLocator = () =>
        {
            var hostWin = new HostWindow
            {
                Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            };
            hostWin.Opened += (_, _) =>
            {
                Console.WriteLine($"[FloatWin] Opened: Content={hostWin.Content?.GetType().Name ?? "null"}");
                // 遍历可视化树，找 DockControl 并设置 Factory
                DumpVisualTree(hostWin, 0);
            };
            return hostWin;
        };
    }

    private static void DumpVisualTree(Avalonia.Visual visual, int depth)
    {
        var indent = new string(' ', depth * 2);
        var type = visual.GetType().Name;
        var extra = "";
        if (visual is global::Dock.Avalonia.Controls.DockControl dc)
        {
            extra = $" Layout={dc.Layout?.Id ?? "null"} Factory={dc.Factory?.GetType().Name ?? "null"}";
            // 设置 Factory
            if (dc.Factory == null && AvaloniaFrontendModule.DockFactory != null)
            {
                dc.Factory = AvaloniaFrontendModule.DockFactory;
                Console.WriteLine($"{indent}[设置 Factory 到 DockControl]");
            }
        }
        Console.WriteLine($"{indent}{type}{extra}");

        if (visual is Avalonia.Controls.Panel panel)
        {
            foreach (var child in panel.Children)
                if (child is Avalonia.Visual v) DumpVisualTree(v, depth + 1);
        }
        else if (visual is Avalonia.Controls.Decorator decorator && decorator.Child is Avalonia.Visual dChild)
        {
            DumpVisualTree(dChild, depth + 1);
        }
        else if (visual is Avalonia.Controls.ContentControl cc && cc.Content is Avalonia.Visual cChild)
        {
            DumpVisualTree(cChild, depth + 1);
        }
        else if (visual is Avalonia.Controls.ItemsControl ic)
        {
            foreach (var item in ic.GetVisualChildren())
                if (item is Avalonia.Visual v) DumpVisualTree(v, depth + 1);
        }
    }

    /// <summary>面板 ID 常量。</summary>
    public static class PanelIds
    {
        public const string SceneBrowser = "SceneBrowser";
        public const string Viewport = "Viewport";
        public const string Inspector = "Inspector";
        public const string ContentBrowser = "ContentBrowser";
        public const string Console = "Console";
        public const string TextureViewerPrefix = "TextureViewer_";
    }

    // 面板引用（供外部设置内容）
    // Viewport 使用 Document（放在 DocumentDock 中），其余使用 Tool（放在 ToolDock 中）
    private Document? _viewport;
    private DocumentDock? _centerDock;
    private Tool? _sceneBrowser;
    private Tool? _inspector;
    private Tool? _contentBrowser;
    private Tool? _console;
    private readonly Dictionary<string, Tool> _textureViewers = new();
    private readonly Dictionary<string, Document> _textureViewerDocuments = new();

    public Document? ViewportPanel => _viewport;
    public DocumentDock? CenterDock => _centerDock;
    public Tool? SceneBrowserPanel => _sceneBrowser;
    public Tool? InspectorPanel => _inspector;
    public Tool? ContentBrowserPanel => _contentBrowser;
    public Tool? ConsolePanel => _console;
    public IReadOnlyDictionary<string, Tool> TextureViewerPanels => _textureViewers;
    public IReadOnlyDictionary<string, Document> TextureViewerDocuments => _textureViewerDocuments;

    /// <summary>
    /// 创建默认编辑器布局。
    /// 使用 Dock 12.0 fluent builder API。
    /// </summary>
    public IRootDock CreateDefaultLayout()
    {
        // 创建面板
        // Viewport 使用 Document（放在 DocumentDock 中），其余使用 Tool（放在 ToolDock 中）
        _viewport = new Document { Id = PanelIds.Viewport, Title = "Viewport", CanFloat = true };
        _sceneBrowser = new Tool { Id = PanelIds.SceneBrowser, Title = "Scene Browser", CanFloat = true };
        _inspector = new Tool { Id = PanelIds.Inspector, Title = "Inspector", CanFloat = true };
        _contentBrowser = new Tool { Id = PanelIds.ContentBrowser, Title = "Content Browser", CanFloat = true };
        _console = new Tool { Id = PanelIds.Console, Title = "Console", CanFloat = true };

        // 右侧上部 ToolDock：SceneBrowser
        var rightTopDock = new ToolDock
        {
            Id = "RightTop",
            Alignment = Alignment.Right,
            Proportion = 0.5,
            ActiveDockable = _sceneBrowser,
            VisibleDockables = CreateList<IDockable>(_sceneBrowser)
        };

        // 右侧下部 ToolDock：Inspector
        var rightBottomDock = new ToolDock
        {
            Id = "RightBottom",
            Alignment = Alignment.Right,
            Proportion = 0.5,
            ActiveDockable = _inspector,
            VisibleDockables = CreateList<IDockable>(_inspector)
        };

        // 右侧组合：SceneBrowser + Inspector 上下排列（占 25% 宽度）
        var rightDock = new ProportionalDock
        {
            Id = "Right",
            Orientation = Orientation.Vertical,
            Proportion = 0.25,
            ActiveDockable = rightTopDock,
            VisibleDockables = CreateList<IDockable>(
                rightTopDock,
                new ProportionalDockSplitter { Id = "SplitRight" },
                rightBottomDock
            )
        };

        // 底部 ToolDock：ContentBrowser + Console
        var bottomDock = new ToolDock
        {
            Id = "Bottom",
            Alignment = Alignment.Bottom,
            Proportion = 0.3,
            ActiveDockable = _contentBrowser,
            VisibleDockables = CreateList<IDockable>(_contentBrowser, _console)
        };

        // 中央 DocumentDock：Viewport（启用标签拖拽浮动）
        _centerDock = new DocumentDock
        {
            Id = "Center",
            ActiveDockable = _viewport,
            VisibleDockables = CreateList<IDockable>(_viewport),
            EnableWindowDrag = true
        };

        // 左侧组合：Viewport + 底部面板（占 75% 宽度）
        var leftDock = new ProportionalDock
        {
            Id = "Left",
            Orientation = Orientation.Vertical,
            Proportion = 0.75,
            ActiveDockable = _centerDock,
            VisibleDockables = CreateList<IDockable>(
                _centerDock,
                new ProportionalDockSplitter { Id = "SplitBottom" },
                bottomDock
            )
        };

        // 根层：左侧 + 右侧（水平排列）
        var rootDock = new ProportionalDock
        {
            Id = "Root",
            Orientation = Orientation.Horizontal,
            ActiveDockable = leftDock,
            VisibleDockables = CreateList<IDockable>(
                leftDock,
                new ProportionalDockSplitter { Id = "Split1" },
                rightDock
            )
        };

        // 根 Dock（启用原生浮动窗口）
        var root = new RootDock
        {
            Id = "Layout",
            ActiveDockable = rootDock,
            VisibleDockables = CreateList<IDockable>(rootDock),
            FloatingWindowHostMode = DockFloatingWindowHostMode.Native,
            Windows = CreateList<IDockWindow>()
        };

        InitLayout(root);

        return root;
    }

    /// <summary>创建纹理查看器面板（浮动 Tool）。</summary>
    public Tool CreateTextureViewerPanel(string assetName, Guid guid)
    {
        var panelId = $"{PanelIds.TextureViewerPrefix}{guid}";
        var tool = new Tool
        {
            Id = panelId,
            Title = $"Texture - {assetName}",
            CanFloat = true,
            CanClose = true,
        };

        _textureViewers[panelId] = tool;
        return tool;
    }

    /// <summary>创建纹理查看器文档页（可停靠到中央文档区）。</summary>
    public Document CreateTextureViewerDocument(string assetName, Guid guid)
    {
        var panelId = $"{PanelIds.TextureViewerPrefix}{guid}";
        var document = new Document
        {
            Id = panelId,
            Title = $"Texture - {assetName}",
            CanFloat = true,
            CanClose = true,
        };

        _textureViewerDocuments[panelId] = document;
        return document;
    }

    public Document? FindDocument(string panelId)
    {
        if (_viewport?.Id == panelId)
        {
            return _viewport;
        }

        return _textureViewerDocuments.TryGetValue(panelId, out var document) ? document : null;
    }

    /// <summary>移除纹理查看器面板。</summary>
    public void RemoveTextureViewerPanel(string panelId)
    {
        _textureViewers.Remove(panelId);
        _textureViewerDocuments.Remove(panelId);
    }
}
