using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
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

    // 面板内容容器（独立于 PanelManager，避免重复挂载）
    private readonly Panel _sceneBrowserContent = new() { Background = new SolidColorBrush(Color.Parse("#FF252526")) };
    private readonly Panel _viewportContent = new() { Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")) };
    private readonly Panel _inspectorContent = new() { Background = new SolidColorBrush(Color.Parse("#FF252526")) };
    private readonly Panel _contentBrowserContent = new() { Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")) };
    private readonly Panel _consoleContent = new() { Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")) };

    public MainEditorWindow()
    {
        InitializeComponent();

        _dockFactory = new EditorDockFactory();

        // 创建默认 Dock 布局
        var layout = _dockFactory.CreateDefaultLayout();

        // 为每个 Document 设置内容容器
        SetDocumentContent(_dockFactory.SceneBrowserPanel, _sceneBrowserContent, "Scene Browser");
        SetDocumentContent(_dockFactory.ViewportPanel, _viewportContent, "Viewport");
        SetDocumentContent(_dockFactory.InspectorPanel, _inspectorContent, "Inspector");
        SetDocumentContent(_dockFactory.ContentBrowserPanel, _contentBrowserContent, "Content Browser");
        SetDocumentContent(_dockFactory.ConsolePanel, _consoleContent, "Console");

        // 设置到 DockControl
        DockControl.Factory = _dockFactory;
        DockControl.Layout = layout;

        // 创建菜单、工具栏、状态栏
        InitializeSubViews();
    }

    /// <summary>
    /// 设置 Document 的内容。
    /// 用 Panel 包装，避免 Avalonia 控件重复挂载。
    /// </summary>
    private void SetDocumentContent(global::Dock.Model.Mvvm.Controls.Document? doc, Panel contentPanel, string title)
    {
        if (doc != null)
        {
            // Document.Context 设置为 Panel，DataTemplate 会渲染它
            doc.Context = contentPanel;
        }
    }

    /// <summary>
    /// 设置面板内容（由 AvaloniaFrontendModule 调用）。
    /// 将 View 添加到对应的 Panel 容器中。
    /// </summary>
    public void SetPanelContent(string panelId, Control content)
    {
        var panel = panelId switch
        {
            EditorDockFactory.PanelIds.SceneBrowser => _sceneBrowserContent,
            EditorDockFactory.PanelIds.Viewport => _viewportContent,
            EditorDockFactory.PanelIds.Inspector => _inspectorContent,
            EditorDockFactory.PanelIds.ContentBrowser => _contentBrowserContent,
            EditorDockFactory.PanelIds.Console => _consoleContent,
            _ => null
        };

        if (panel != null)
        {
            panel.Children.Clear();
            panel.Children.Add(content);
            Console.WriteLine($"[MainEditorWindow] 面板内容已设置: {panelId}");
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
            Width = 28,
            Height = 28,
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
