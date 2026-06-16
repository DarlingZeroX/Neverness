using Dock.Avalonia.Controls;
using Dock.Model;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.Mvvm;
using Dock.Model.Mvvm.Controls;
using Dock.Model.Mvvm.Core;

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
        DefaultHostWindowLocator = () => new HostWindow();
    }

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
    // Viewport 使用 Document（放在 DocumentDock 中），其余使用 Tool（放在 ToolDock 中）
    private Document? _viewport;
    private Tool? _sceneBrowser;
    private Tool? _inspector;
    private Tool? _contentBrowser;
    private Tool? _console;

    public Document? ViewportPanel => _viewport;
    public Tool? SceneBrowserPanel => _sceneBrowser;
    public Tool? InspectorPanel => _inspector;
    public Tool? ContentBrowserPanel => _contentBrowser;
    public Tool? ConsolePanel => _console;

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
            VisibleDockables = CreateList<IDockable>(_sceneBrowser),
            EnableWindowDrag = true
        };

        // 右侧下部 ToolDock：Inspector
        var rightBottomDock = new ToolDock
        {
            Id = "RightBottom",
            Alignment = Alignment.Right,
            Proportion = 0.5,
            ActiveDockable = _inspector,
            VisibleDockables = CreateList<IDockable>(_inspector),
            EnableWindowDrag = true
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
            VisibleDockables = CreateList<IDockable>(_contentBrowser, _console),
            EnableWindowDrag = true
        };

        // 中央 DocumentDock：Viewport（启用标签拖拽浮动）
        var centerDock = new DocumentDock
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
            ActiveDockable = centerDock,
            VisibleDockables = CreateList<IDockable>(
                centerDock,
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
}
