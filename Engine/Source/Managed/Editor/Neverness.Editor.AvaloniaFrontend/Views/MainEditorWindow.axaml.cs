using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Dock.Avalonia.Controls;
using Neverness.Editor.AvaloniaFrontend.Dock;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

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

    /// <summary>
    /// 刷新菜单栏（在 CoreModuleImp.Install() 之后调用）。
    /// 解决时序问题：MainEditorWindow 在菜单贡献者注册之前创建。
    /// </summary>
    public void RefreshMenuBar()
    {
        InitializeMenuBar();
    }

    /// <summary>
    /// 从 EditorMenuRegistry 读取菜单树，渲染为 Avalonia Menu。
    /// </summary>
    private void InitializeMenuBar()
    {
        // 从 EditorMenuRegistry 获取菜单树
        var tree = MenuRegistryImp.Instance.GetTree();

        // 清空现有菜单项
        MenuBar.Items.Clear();

        // 设置菜单栏样式
        MenuBar.Background = new SolidColorBrush(Color.Parse("#FF2D2D30"));
        MenuBar.Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC"));

        // 遍历菜单树的根节点，创建顶级菜单
        foreach (var rootNode in tree.SortedRoots)
        {
            var menuItem = CreateMenuFromNode(rootNode);
            if (menuItem != null)
            {
                MenuBar.Items.Add(menuItem);
            }
        }
    }

    /// <summary>
    /// 从 MenuTreeNode 递归创建 Avalonia MenuItem。
    /// </summary>
    private MenuItem? CreateMenuFromNode(MenuTreeNode node)
    {
        // 分隔符
        if (node.IsSeparator)
        {
            return null;
        }

        var menuItem = new MenuItem
        {
            Header = node.Name,
        };

        // 如果是叶子节点（有绑定的菜单项）
        if (node.IsLeaf && node.Item.HasValue)
        {
            var item = node.Item.Value;

            // 设置快捷键
            if (!string.IsNullOrEmpty(item.Shortcut))
            {
                menuItem.InputGesture = ParseKeyGesture(item.Shortcut);
            }

            // 设置图标
            if (!string.IsNullOrEmpty(item.Icon))
            {
                menuItem.Icon = new TextBlock
                {
                    Text = item.Icon,
                    FontSize = 14,
                };
            }

            // 绑定命令
            if (item.Command != null)
            {
                var command = item.Command;
                menuItem.Click += (_, _) =>
                {
                    try
                    {
                        command.Execute(new EditorCommandContext());
                    }
                    catch (Exception ex)
                    {
                        Console.Error.WriteLine($"[MainEditorWindow] 命令执行失败 '{command.Id}': {ex.Message}");
                    }
                };

                // 设置 CanExecute
                if (command.CanExecute != null)
                {
                    menuItem.IsEnabled = command.CanExecute();
                }

                // 设置 IsChecked
                if (command.IsChecked != null)
                {
                    menuItem.IsChecked = command.IsChecked();
                }
            }
            else
            {
                // 没有绑定命令的菜单项，使用路径作为命令 ID
                var path = item.Path;
                menuItem.Click += (_, _) =>
                {
                    try
                    {
                        EditorMenuRegistry.ExecuteCommand(path);
                    }
                    catch (Exception ex)
                    {
                        Console.Error.WriteLine($"[MainEditorWindow] 命令执行失败 '{path}': {ex.Message}");
                    }
                };
            }

            // 动态菜单
            if (node.IsDynamic && node.DynamicBuilder != null)
            {
                menuItem.AttachedToVisualTree += (_, _) =>
                {
                    // 动态生成子菜单项
                    var builder = new DynamicMenuBuilder();
                    node.DynamicBuilder(builder);
                    var dynamicItems = builder.Build();

                    menuItem.Items.Clear();
                    foreach (var dynamicItem in dynamicItems)
                    {
                        if (dynamicItem.IsSeparator)
                        {
                            menuItem.Items.Add(CreateSeparator());
                        }
                        else
                        {
                            var childMenuItem = CreateMenuItemFromEditorMenuItem(dynamicItem);
                            if (childMenuItem != null)
                            {
                                menuItem.Items.Add(childMenuItem);
                            }
                        }
                    }
                };
            }
        }
        else
        {
            // 非叶子节点，递归创建子菜单
            foreach (var childNode in node.SortedChildren)
            {
                if (childNode.IsSeparator)
                {
                    menuItem.Items.Add(CreateSeparator());
                }
                else
                {
                    var childMenuItem = CreateMenuFromNode(childNode);
                    if (childMenuItem != null)
                    {
                        menuItem.Items.Add(childMenuItem);
                    }
                }
            }
        }

        return menuItem;
    }

    /// <summary>
    /// 从 EditorMenuItem 创建 Avalonia MenuItem。
    /// </summary>
    private MenuItem? CreateMenuItemFromEditorMenuItem(EditorMenuItem item)
    {
        if (item.IsSeparator)
        {
            return null;
        }

        var menuItem = new MenuItem
        {
            Header = item.Path.Split('/').Last(),
        };

        // 设置快捷键
        if (!string.IsNullOrEmpty(item.Shortcut))
        {
            menuItem.InputGesture = ParseKeyGesture(item.Shortcut);
        }

        // 设置图标
        if (!string.IsNullOrEmpty(item.Icon))
        {
            menuItem.Icon = new TextBlock
            {
                Text = item.Icon,
                FontSize = 14,
            };
        }

        // 绑定命令
        if (item.Command != null)
        {
            var command = item.Command;
            menuItem.Click += (_, _) =>
            {
                try
                {
                    command.Execute(new EditorCommandContext());
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine($"[MainEditorWindow] 命令执行失败 '{command.Id}': {ex.Message}");
                }
            };

            // 设置 CanExecute
            if (command.CanExecute != null)
            {
                menuItem.IsEnabled = command.CanExecute();
            }

            // 设置 IsChecked
            if (command.IsChecked != null)
            {
                menuItem.IsChecked = command.IsChecked();
            }
        }
        else
        {
            // 没有绑定命令的菜单项，使用路径作为命令 ID
            var path = item.Path;
            menuItem.Click += (_, _) =>
            {
                try
                {
                    EditorMenuRegistry.ExecuteCommand(path);
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine($"[MainEditorWindow] 命令执行失败 '{path}': {ex.Message}");
                }
            };
        }

        return menuItem;
    }

    /// <summary>
    /// 创建分隔符。
    /// </summary>
    private static Avalonia.Controls.MenuItem CreateSeparator()
    {
        return new Avalonia.Controls.MenuItem
        {
            Header = "-",
            IsEnabled = false,
            Height = 1,
            Padding = new Thickness(0),
        };
    }

    /// <summary>
    /// 解析快捷键字符串为 KeyGesture。
    /// </summary>
    private static KeyGesture? ParseKeyGesture(string shortcut)
    {
        try
        {
            return KeyGesture.Parse(shortcut);
        }
        catch
        {
            Console.Error.WriteLine($"[MainEditorWindow] 无法解析快捷键: {shortcut}");
            return null;
        }
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
        EditorMenuRegistry.ExecuteCommand(commandId);
    }
}
