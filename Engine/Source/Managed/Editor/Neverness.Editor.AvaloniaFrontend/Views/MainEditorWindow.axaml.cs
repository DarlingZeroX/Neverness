using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Dock.Avalonia.Controls;
using Dock.Model.Core;
using Dock.Model.Mvvm.Controls;
using Neverness.Editor.AvaloniaFrontend.Dock;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

// 窗口边缘方向枚举（用于 resize 拖拽）
using WindowEdge = Avalonia.Controls.WindowEdge;

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

        // 保存 Factory 到静态字段（供浮动窗口 DataTemplate 使用）
        AvaloniaFrontendModule.DockFactory = _dockFactory;

        // 创建默认 Dock 布局
        var layout = _dockFactory.CreateDefaultLayout();

        // 设置到 DockControl
        App.ConfigureDockControl(DockControl, _dockFactory, layout);

        // DataTemplate 已在 App.axaml.cs 中全局注册（Application.DataTemplates）
        // 浮动窗口是独立 HostWindow，不继承主窗口 DockControl 的 DataTemplates

        // 配置浮动窗口工厂（Native 模式，原生 OS 窗口）
        DockControl.HostWindowFactory = App.CreateDockHostWindow;

        // 保存 DockControl 到静态字段（供 TextureAssetOpener 等使用）
        AvaloniaFrontendModule.MainDockControl = DockControl;

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

    public bool TryActivateDocument(string panelId)
    {
        var document = _dockFactory.FindDocument(panelId);
        var centerDock = _dockFactory.CenterDock;
        if (document == null || centerDock == null)
        {
            return false;
        }

        if (document.Owner is null)
        {
            centerDock.AddDocument(document);
        }

        _dockFactory.SetActiveDockable(document);
        return true;
    }

    public Document ShowDocument(Document document, Control content)
    {
        var centerDock = _dockFactory.CenterDock
            ?? throw new InvalidOperationException("Center document dock is not initialized.");

        document.Context = content;

        if (document.Owner is null)
        {
            centerDock.AddDocument(document);
        }

        _dockFactory.SetActiveDockable(document);
        return document;
    }

    public bool RemoveDocument(string panelId)
    {
        var document = _dockFactory.FindDocument(panelId);
        if (document == null)
        {
            return false;
        }

        _dockFactory.RemoveDockable(document, true);
        return true;
    }

    private void InitializeSubViews()
    {
        InitializeMenuBar();
        InitializeToolbar();
        InitializeStatusBar();
        InitializeWindowControls();
        InitializeResizeHandles();
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
        // Save/Undo/Redo 靠左
        var leftPanel = new StackPanel
        {
            Orientation = Avalonia.Layout.Orientation.Horizontal,
            Spacing = 0,
            HorizontalAlignment = HorizontalAlignment.Left,
        };
        AddToolButtonTo(leftPanel, "💾", "Save", () => ExecuteCommand("file.save"));
        AddToolButtonTo(leftPanel, "↩", "Undo", () => ExecuteCommand("edit.undo"));
        AddToolButtonTo(leftPanel, "↪", "Redo", () => ExecuteCommand("edit.redo"));

        // Play/Pause/Stop 居中
        var centerPanel = new StackPanel
        {
            Orientation = Avalonia.Layout.Orientation.Horizontal,
            Spacing = 0,
            HorizontalAlignment = HorizontalAlignment.Center,
        };
        AddToolButtonTo(centerPanel, "▶", "Play", () => ExecuteCommand("scene.play"));
        AddToolButtonTo(centerPanel, "⏸", "Pause", () => ExecuteCommand("scene.pause"));
        AddToolButtonTo(centerPanel, "⏹", "Stop", () => ExecuteCommand("scene.stop"));

        ToolbarPanel.Children.Add(leftPanel);
        ToolbarPanel.Children.Add(centerPanel);
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

    private void AddToolButtonTo(Panel target, string icon, string tooltip, Action onClick)
    {
        var iconText = new TextBlock
        {
            Text = icon,
            FontSize = 20,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };

        var border = new Border
        {
            Width = 40,
            Height = 40,
            Background = Brushes.Transparent,
            Margin = new Thickness(0),
            Padding = new Thickness(0),
            Child = iconText,
            Cursor = new Cursor(StandardCursorType.Hand),
        };
        ToolTip.SetTip(border, tooltip);
        border.PointerPressed += (_, _) => onClick();
        border.PointerEntered += (_, _) => border.Background = new SolidColorBrush(Color.Parse("#FF3C3C3C"));
        border.PointerExited += (_, _) => border.Background = Brushes.Transparent;
        target.Children.Add(border);
    }

    private void ExecuteCommand(string commandId)
    {
        Console.WriteLine($"[MainEditorWindow] 命令: {commandId}");
        EditorMenuRegistry.ExecuteCommand(commandId);
    }

    /// <summary>
    /// 初始化窗口控制按钮（最小化、最大化、关闭）。
    /// </summary>
    private void InitializeWindowControls()
    {
        // 最小化
        MinimizeButton.Click += (_, _) => WindowState = WindowState.Minimized;

        // 最大化/还原切换
        MaximizeButton.Click += (_, _) =>
        {
            WindowState = WindowState == WindowState.Maximized
                ? WindowState.Normal
                : WindowState.Maximized;
        };

        // 关闭
        CloseButton.Click += (_, _) => Close();

        // 拖动窗口（按下鼠标左键时开始拖动）
        TitleBar.PointerPressed += (_, e) =>
        {
            if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
            {
                BeginMoveDrag(e);
            }
        };

        // 双击标题栏最大化/还原
        TitleBar.DoubleTapped += (_, _) =>
        {
            WindowState = WindowState == WindowState.Maximized
                ? WindowState.Normal
                : WindowState.Maximized;
        };
    }

    /// <summary>
    /// 初始化 resize 手柄（8个边角）。
    /// </summary>
    private void InitializeResizeHandles()
    {
        // 四个角
        SetupSide(TopLeft, StandardCursorType.TopLeftCorner, WindowEdge.NorthWest);
        SetupSide(TopRight, StandardCursorType.TopRightCorner, WindowEdge.NorthEast);
        SetupSide(BottomLeft, StandardCursorType.BottomLeftCorner, WindowEdge.SouthWest);
        SetupSide(BottomRight, StandardCursorType.BottomRightCorner, WindowEdge.SouthEast);

        // 四条边
        SetupSide(Top, StandardCursorType.TopSide, WindowEdge.North);
        SetupSide(Bottom, StandardCursorType.BottomSide, WindowEdge.South);
        SetupSide(Left, StandardCursorType.LeftSide, WindowEdge.West);
        SetupSide(Right, StandardCursorType.RightSide, WindowEdge.East);
    }

    /// <summary>
    /// 设置 resize 手柄的拖拽行为。
    /// </summary>
    private void SetupSide(Control ctl, StandardCursorType cursor, WindowEdge edge)
    {
        ctl.Cursor = new Cursor(cursor);
        ctl.PointerPressed += (_, e) =>
        {
            // 仅在窗口未最大化时允许 resize
            if (WindowState == WindowState.Normal)
            {
                BeginResizeDrag(edge, e);
            }
        };
    }
}
