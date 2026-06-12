using Avalonia.Controls;
using Avalonia.Input;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 主菜单栏 Avalonia View——渲染编辑器主菜单。
///
/// 与 ImGui 的 ImGuiMenuBar 对应。
/// 从 EditorMenuRegistry 读取菜单定义，渲染为 Avalonia Menu。
/// </summary>
public class MenuBarAvaloniaView : AvaloniaViewBase
{
    private Menu? _menuBar;

    public MenuBarAvaloniaView() : base("MenuBar")
    {
    }

    public override Type ViewModelType => typeof(object); // 菜单栏没有 ViewModel

    public override void Bind(object viewModel)
    {
        // 菜单栏不绑定 ViewModel，直接从 EditorMenuRegistry 读取
        CreateMenuBar();
    }

    public override void Unbind()
    {
        _menuBar = null;
    }

    /// <summary>获取菜单栏控件。</summary>
    public Menu? GetMenuBar() => _menuBar;

    /// <summary>创建菜单栏。</summary>
    private void CreateMenuBar()
    {
        _menuBar = new Menu
        {
            Background = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF2D2D30")),
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FFCCCCCC")),
        };

        // 从 EditorMenuRegistry 读取菜单定义
        // TODO: 实现菜单注册表读取
        // 当前创建默认菜单结构

        // 文件菜单
        var fileMenu = CreateMenuItem("File", new Control[]
        {
            CreateMenuItem("New Scene", "Ctrl+N", () => ExecuteCommand("file.new")),
            CreateMenuItem("Open Scene", "Ctrl+O", () => ExecuteCommand("file.open")),
            CreateSeparator(),
            CreateMenuItem("Save", "Ctrl+S", () => ExecuteCommand("file.save")),
            CreateMenuItem("Save As...", "Ctrl+Shift+S", () => ExecuteCommand("file.saveAs")),
            CreateSeparator(),
            CreateMenuItem("Exit", "Alt+F4", () => ExecuteCommand("file.exit")),
        });
        _menuBar.Items.Add(fileMenu);

        // 编辑菜单
        var editMenu = CreateMenuItem("Edit", new Control[]
        {
            CreateMenuItem("Undo", "Ctrl+Z", () => ExecuteCommand("edit.undo")),
            CreateMenuItem("Redo", "Ctrl+Y", () => ExecuteCommand("edit.redo")),
            CreateSeparator(),
            CreateMenuItem("Cut", "Ctrl+X", () => ExecuteCommand("edit.cut")),
            CreateMenuItem("Copy", "Ctrl+C", () => ExecuteCommand("edit.copy")),
            CreateMenuItem("Paste", "Ctrl+V", () => ExecuteCommand("edit.paste")),
            CreateMenuItem("Delete", "Del", () => ExecuteCommand("edit.delete")),
        });
        _menuBar.Items.Add(editMenu);

        // 视图菜单
        var viewMenu = CreateMenuItem("View", new Control[]
        {
            CreateMenuItem("Scene Browser", null, () => TogglePanel("SceneBrowser")),
            CreateMenuItem("Content Browser", null, () => TogglePanel("ContentBrowser")),
            CreateMenuItem("Inspector", null, () => TogglePanel("Inspector")),
            CreateMenuItem("Console", null, () => TogglePanel("Console")),
            CreateMenuItem("Viewport", null, () => TogglePanel("Viewport")),
        });
        _menuBar.Items.Add(viewMenu);

        // 场景菜单
        var sceneMenu = CreateMenuItem("Scene", new Control[]
        {
            CreateMenuItem("Play", "F5", () => ExecuteCommand("scene.play")),
            CreateMenuItem("Pause", "F6", () => ExecuteCommand("scene.pause")),
            CreateMenuItem("Stop", "F7", () => ExecuteCommand("scene.stop")),
            CreateSeparator(),
            CreateMenuItem("Create Entity", null, () => ExecuteCommand("entity.create")),
        });
        _menuBar.Items.Add(sceneMenu);

        // 工具菜单
        var toolsMenu = CreateMenuItem("Tools", new Control[]
        {
            CreateMenuItem("Settings", null, () => ExecuteCommand("tools.settings")),
        });
        _menuBar.Items.Add(toolsMenu);

        // 帮助菜单
        var helpMenu = CreateMenuItem("Help", new Control[]
        {
            CreateMenuItem("About", null, () => ExecuteCommand("help.about")),
        });
        _menuBar.Items.Add(helpMenu);
    }

    /// <summary>创建菜单项。</summary>
    private MenuItem CreateMenuItem(string header, object? inputGesture, Action? onClick)
    {
        var menuItem = new MenuItem
        {
            Header = header,
        };

        if (inputGesture is string gesture)
        {
            // TODO: 设置快捷键手势
        }

        if (onClick != null)
        {
            menuItem.Click += (_, _) => onClick();
        }

        return menuItem;
    }

    /// <summary>创建带子菜单的菜单项。</summary>
    private MenuItem CreateMenuItem(string header, Control[] children)
    {
        var menuItem = new MenuItem
        {
            Header = header,
        };

        foreach (var child in children)
        {
            menuItem.Items.Add(child);
        }

        return menuItem;
    }

    /// <summary>创建分隔线。</summary>
    private static Separator CreateSeparator()
    {
        return new Separator();
    }

    /// <summary>执行命令。</summary>
    private void ExecuteCommand(string commandId)
    {
        Console.WriteLine($"[MenuBar] 执行命令: {commandId}");
        // TODO: 通过 EditorCommandRegistry 执行命令
    }

    /// <summary>切换面板显示/隐藏。</summary>
    private void TogglePanel(string panelId)
    {
        Console.WriteLine($"[MenuBar] 切换面板: {panelId}");
        // TODO: 通过 PanelManager 切换面板
    }
}
