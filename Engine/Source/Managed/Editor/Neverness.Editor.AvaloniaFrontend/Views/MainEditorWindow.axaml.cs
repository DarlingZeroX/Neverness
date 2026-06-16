using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Layout;
using Avalonia.Media;
using Dock.Avalonia.Controls;
using Neverness.Editor.AvaloniaFrontend.Dock;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 主编辑器窗口——Avalonia 版本。
///
/// 集成 Dock 布局、菜单栏、工具栏、状态栏。
/// 面板内容容器在此管理，不通过 PanelManager。
/// </summary>
public partial class MainEditorWindow : Window
{
    private readonly EditorDockFactory _dockFactory;

    public MainEditorWindow()
    {
        InitializeComponent();

        _dockFactory = new EditorDockFactory();

        // 创建默认 Dock 布局
        var layout = _dockFactory.CreateDefaultLayout();

        // 设置到 DockControl
        DockControl.Factory = _dockFactory;
        DockControl.Layout = layout;

        // Document 模板：渲染 Document.Context（Viewport 专用）
        DockControl.DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Document),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.Document doc && doc.Context is Control ctrl)
                    return ctrl;
                return new TextBlock { Text = "No Content" };
            }));

        // Tool 模板：渲染 Tool.Context（其余面板使用）
        DockControl.DataTemplates.Insert(1, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Tool),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.Tool tool && tool.Context is Control ctrl)
                    return ctrl;
                return new TextBlock { Text = "No Content" };
            }));

        // 配置浮动窗口工厂（Native 模式，原生 OS 窗口）
        DockControl.HostWindowFactory = () => new HostWindow
        {
            IsToolWindow = true,
            ToolChromeControlsWholeWindow = true,
        };

        // 创建菜单、工具栏、状态栏
        InitializeSubViews();
    }

    /// <summary>
    /// 设置 Document 的内容。
    /// 用 Panel 包装，避免 Avalonia 控件重复挂载。
    /// </summary>
    /// <summary>
    /// 设置面板内容（由 AvaloniaFrontendModule 调用）。
    /// 直接设置 Document.Context 为视图，FuncDataTemplate 的绑定会自动渲染。
    /// </summary>
    public void SetPanelContent(string panelId, Control content)
    {
        Console.WriteLine($"[MainEditorWindow] SetPanelContent 调用: {panelId}, 内容类型: {content.GetType().Name}");

        // Viewport 是 Document，其余面板是 Tool
        if (panelId == EditorDockFactory.PanelIds.Viewport)
        {
            var doc = _dockFactory.ViewportPanel;
            if (doc != null)
            {
                doc.Context = content;
                Console.WriteLine($"[MainEditorWindow] Document.Context 已设置: {panelId}");
            }
            else
            {
                Console.Error.WriteLine($"[MainEditorWindow] 未找到 Document: {panelId}");
            }
        }
        else
        {
            var tool = panelId switch
            {
                EditorDockFactory.PanelIds.SceneBrowser => _dockFactory.SceneBrowserPanel,
                EditorDockFactory.PanelIds.Inspector => _dockFactory.InspectorPanel,
                EditorDockFactory.PanelIds.ContentBrowser => _dockFactory.ContentBrowserPanel,
                EditorDockFactory.PanelIds.Console => _dockFactory.ConsolePanel,
                _ => null
            };

            if (tool != null)
            {
                tool.Context = content;
                Console.WriteLine($"[MainEditorWindow] Tool.Context 已设置: {panelId}");
            }
            else
            {
                Console.Error.WriteLine($"[MainEditorWindow] 未找到 Tool: {panelId}");
            }
        }
    }

    private void InitializeSubViews()
    {
        InitializeMenuBar();
        InitializeToolbar();
        InitializeStatusBar();
    }

    private void InitializeMenuBar()
    {
        var fileMenu = new MenuItem { Header = "File" };
        fileMenu.Items.Add(CreateMenuItem("New Scene", () => ExecuteCommand("file.new")));
        fileMenu.Items.Add(CreateMenuItem("Open Scene", () => ExecuteCommand("file.open")));
        fileMenu.Items.Add(new Separator());
        fileMenu.Items.Add(CreateMenuItem("Save", () => ExecuteCommand("file.save")));
        fileMenu.Items.Add(new Separator());
        fileMenu.Items.Add(CreateMenuItem("Exit", () => ExecuteCommand("file.exit")));
        MenuBar.Items.Add(fileMenu);

        var editMenu = new MenuItem { Header = "Edit" };
        editMenu.Items.Add(CreateMenuItem("Undo", () => ExecuteCommand("edit.undo")));
        editMenu.Items.Add(CreateMenuItem("Redo", () => ExecuteCommand("edit.redo")));
        editMenu.Items.Add(new Separator());
        editMenu.Items.Add(CreateMenuItem("Cut", () => ExecuteCommand("edit.cut")));
        editMenu.Items.Add(CreateMenuItem("Copy", () => ExecuteCommand("edit.copy")));
        editMenu.Items.Add(CreateMenuItem("Paste", () => ExecuteCommand("edit.paste")));
        MenuBar.Items.Add(editMenu);

        var sceneMenu = new MenuItem { Header = "Scene" };
        sceneMenu.Items.Add(CreateMenuItem("Play", () => ExecuteCommand("scene.play")));
        sceneMenu.Items.Add(CreateMenuItem("Pause", () => ExecuteCommand("scene.pause")));
        sceneMenu.Items.Add(CreateMenuItem("Stop", () => ExecuteCommand("scene.stop")));
        MenuBar.Items.Add(sceneMenu);

        var helpMenu = new MenuItem { Header = "Help" };
        helpMenu.Items.Add(CreateMenuItem("About", () => ExecuteCommand("help.about")));
        MenuBar.Items.Add(helpMenu);
    }

    private void InitializeToolbar()
    {
        AddToolButton("▶", "Play", () => ExecuteCommand("scene.play"));
        AddToolButton("⏸", "Pause", () => ExecuteCommand("scene.pause"));
        AddToolButton("⏹", "Stop", () => ExecuteCommand("scene.stop"));
        AddToolButton("💾", "Save", () => ExecuteCommand("file.save"));
        AddToolButton("↩", "Undo", () => ExecuteCommand("edit.undo"));
        AddToolButton("↪", "Redo", () => ExecuteCommand("edit.redo"));
    }

    private void InitializeStatusBar()
    {
        var statusText = new TextBlock
        {
            Text = "Ready",
            FontSize = 11,
            Foreground = Brushes.White,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0),
        };
        DockPanel.SetDock(statusText, Avalonia.Controls.Dock.Left);
        StatusBar.Children.Add(statusText);
    }

    private MenuItem CreateMenuItem(string header, Action onClick)
    {
        var item = new MenuItem { Header = header };
        item.Click += (_, _) => onClick();
        return item;
    }

    private void AddToolButton(string icon, string tooltip, Action onClick)
    {
        var button = new Button
        {
            Content = icon,
            MinWidth = 28,
            MinHeight = 28,
            Padding = new Thickness(4),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            FontSize = 14,
        };
        ToolTip.SetTip(button, tooltip);
        button.Click += (_, _) => onClick();
        button.PointerEntered += (_, _) => button.Background = new SolidColorBrush(Color.Parse("#FF3C3C3C"));
        button.PointerExited += (_, _) => button.Background = Brushes.Transparent;
        ToolbarPanel.Children.Add(button);
    }

    private void ExecuteCommand(string commandId)
    {
        Console.WriteLine($"[MainEditorWindow] 命令: {commandId}");
    }
}
