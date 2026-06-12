using Dock.Model;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.Mvvm;
using Dock.Model.Mvvm.Controls;

namespace Neverness.Editor.AvaloniaFrontend.Dock;

/// <summary>
/// 编辑器 Dock 工厂——创建默认编辑器布局。
///
/// 参考 Dock 官方 DockCodeOnlyMvvmSample。
///
/// 默认布局：
/// ┌──────────┬───────────────────┬──────────────────┐
/// │  Scene   │                   │   Inspector      │
/// │ Browser  │    Viewport       │                  │
/// ├──────────┴───────────────────┴──────────────────┤
/// │  Content Browser    │  Console                  │
/// └─────────────────────┴──────────────────────────┘
/// </summary>
public class EditorDockFactory : Factory
{
    /// <summary>面板 ID 常量。</summary>
    public static class PanelIds
    {
        public const string SceneBrowser = "SceneBrowser";
        public const string Viewport = "Viewport";
        public const string Inspector = "Inspector";
        public const string ContentBrowser = "ContentBrowser";
        public const string Console = "Console";
    }

    // 面板引用（供外部设置内容）
    private Document? _sceneBrowser;
    private Document? _viewport;
    private Document? _inspector;
    private Document? _contentBrowser;
    private Document? _console;

    public Document? SceneBrowserPanel => _sceneBrowser;
    public Document? ViewportPanel => _viewport;
    public Document? InspectorPanel => _inspector;
    public Document? ContentBrowserPanel => _contentBrowser;
    public Document? ConsolePanel => _console;

    /// <summary>
    /// 创建默认编辑器布局。
    /// 使用 Dock 12.0 fluent builder API。
    /// </summary>
    public IRootDock CreateDefaultLayout()
    {
        // 创建面板
        _sceneBrowser = new Document { Id = PanelIds.SceneBrowser, Title = "Scene Browser" };
        _viewport = new Document { Id = PanelIds.Viewport, Title = "Viewport" };
        _inspector = new Document { Id = PanelIds.Inspector, Title = "Inspector" };
        _contentBrowser = new Document { Id = PanelIds.ContentBrowser, Title = "Content Browser" };
        _console = new Document { Id = PanelIds.Console, Title = "Console" };

        // 左侧 ToolDock：SceneBrowser
        var leftDock = new ToolDock
        {
            Id = "Left",
            Alignment = Alignment.Left,
            Proportion = 0.2,
            ActiveDockable = _sceneBrowser,
            VisibleDockables = CreateList<IDockable>(_sceneBrowser)
        };

        // 右侧 ToolDock：Inspector
        var rightDock = new ToolDock
        {
            Id = "Right",
            Alignment = Alignment.Right,
            Proportion = 0.25,
            ActiveDockable = _inspector,
            VisibleDockables = CreateList<IDockable>(_inspector)
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

        // 中央 DocumentDock：Viewport
        var centerDock = new DocumentDock
        {
            Id = "Center",
            ActiveDockable = _viewport,
            VisibleDockables = CreateList<IDockable>(_viewport)
        };

        // 中间层：左侧 + 中央 + 右侧
        var middleDock = new ProportionalDock
        {
            Id = "Middle",
            Orientation = Orientation.Horizontal,
            ActiveDockable = centerDock,
            VisibleDockables = CreateList<IDockable>(
                leftDock,
                new ProportionalDockSplitter { Id = "Split1" },
                centerDock,
                new ProportionalDockSplitter { Id = "Split2" },
                rightDock
            )
        };

        // 根层：中间 + 底部
        var rootDock = new ProportionalDock
        {
            Id = "Root",
            Orientation = Orientation.Vertical,
            ActiveDockable = middleDock,
            VisibleDockables = CreateList<IDockable>(
                middleDock,
                new ProportionalDockSplitter { Id = "Split3" },
                bottomDock
            )
        };

        // 根 Dock
        var root = new RootDock
        {
            Id = "Layout",
            ActiveDockable = rootDock,
            VisibleDockables = CreateList<IDockable>(rootDock)
        };

        InitLayout(root);

        return root;
    }
}
