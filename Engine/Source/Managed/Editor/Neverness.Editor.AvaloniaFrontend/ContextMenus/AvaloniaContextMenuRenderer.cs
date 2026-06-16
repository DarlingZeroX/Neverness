using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.ContextMenus;

/// <summary>
/// Avalonia 上下文菜单渲染器——从 ContextMenuManager 读取 EditorMenuItem，渲染为 Avalonia ContextMenu。
///
/// 与 ImGui 的 ImGuiContextMenuRenderer 对应。
/// 实现 IContextMenuRenderer 接口，使 Core/Assets 模块不依赖具体 UI 框架。
/// </summary>
public class AvaloniaContextMenuRenderer : IContextMenuRenderer
{
    // 当前活跃的上下文菜单
    private ContextMenu? _activeMenu;

    /// <summary>开始窗口级弹出菜单（右键空白区域）。</summary>
    public bool BeginWindowPopup(string contextId)
    {
        _activeMenu = CreateStyledMenu();
        return true;
    }

    /// <summary>开始项目级弹出菜单（右键某个项目）。</summary>
    public bool BeginItemPopup(string contextId)
    {
        return BeginWindowPopup(contextId);
    }

    /// <summary>结束弹出菜单。</summary>
    public void EndPopup()
    {
        // 菜单在 Open() 后自动管理生命周期
        _activeMenu = null;
    }

    /// <summary>在弹出菜单内渲染所有已注册的内容。</summary>
    public void RenderPopupContent(string contextId, ContextMenuManager manager)
    {
        if (_activeMenu == null) return;

        manager.EnsureContributors();

        // 收集所有菜单项（静态 + 回调），按 SortOrder 排序
        var allItems = new List<EditorMenuItem>();

        var staticItems = manager.GetStaticItems(contextId);
        if (staticItems != null)
            allItems.AddRange(staticItems);

        var callbacks = manager.GetCallbacks(contextId);
        if (callbacks != null)
        {
            foreach (var callback in callbacks)
            {
                var builder = new ContextMenuBuilder();
                callback(builder);
                allItems.AddRange(builder.Build());
            }
        }

        // 按 SortOrder 排序后，构建层级菜单
        foreach (var item in allItems.OrderBy(i => i.SortOrder))
        {
            if (item.IsSeparator)
            {
                _activeMenu.Items.Add(CreateSeparator());
                continue;
            }

            var segments = item.Path.Split('/', StringSplitOptions.RemoveEmptyEntries);

            if (segments.Length <= 1)
            {
                // 扁平项，直接添加
                _activeMenu.Items.Add(ConvertToAvaloniaMenuItem(item));
            }
            else
            {
                // 层级项：找到或创建子菜单容器，最后一段是实际菜单项
                var parent = FindOrCreateSubmenu(_activeMenu, segments.AsSpan(0, segments.Length - 1));
                var leafItem = ConvertToAvaloniaMenuItem(item with { Path = segments[^1] });
                parent.Items.Add(leafItem);
            }
        }
    }

    /// <summary>完整渲染窗口级上下文菜单（Begin → 内容 → End）。</summary>
    public void RenderWindowContextMenu(string contextId, ContextMenuManager manager)
    {
        if (BeginWindowPopup(contextId))
        {
            RenderPopupContent(contextId, manager);
            EndPopup();
        }
    }

    /// <summary>完整渲染项目级上下文菜单（Begin → 内容 → End）。</summary>
    public void RenderItemContextMenu(string contextId, ContextMenuManager manager)
    {
        if (BeginItemPopup(contextId))
        {
            RenderPopupContent(contextId, manager);
            EndPopup();
        }
    }

    /// <summary>
    /// 在指定控件上构建并显示上下文菜单。
    /// 这是 Avalonia 端的主要入口——ContextMenuManager 注册的菜单项由此方法渲染。
    /// </summary>
    public ContextMenu? BuildAndShow(string contextId, ContextMenuManager manager, Control target)
    {
        // 确保所有贡献者已构建（可能在 RegisterContributor 之后才首次调用）
        manager.EnsureContributors();

        // 构建菜单（Begin → 渲染内容）
        BeginWindowPopup(contextId);
        RenderPopupContent(contextId, manager);

        // 保存引用后再清理
        var menu = _activeMenu;
        _activeMenu = null;

        if (menu != null && menu.Items.Count > 0)
        {
            menu.Open(target);
            return menu;
        }

        return null;
    }

    // ================= 内部方法 =================

    /// <summary>创建带样式的 ContextMenu。</summary>
    private static ContextMenu CreateStyledMenu()
    {
        return new ContextMenu
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            BorderBrush = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            BorderThickness = new Thickness(1),
            MinWidth = 180,
        };
    }

    /// <summary>创建带样式的 Separator。</summary>
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
    /// 在 parent 容器中查找或创建子菜单路径。
    /// 例如 segments = ["Create", "Rendering"] 会在 parent 下找 "Create" 子菜单，
    /// 然后在 "Create" 下找 "Rendering" 子菜单，返回 "Rendering" 的 MenuItem。
    /// </summary>
    private static Avalonia.Controls.MenuItem FindOrCreateSubmenu(ItemsControl parent, ReadOnlySpan<string> segments)
    {
        foreach (var segment in segments)
        {
            var existing = parent.Items
                .OfType<Avalonia.Controls.MenuItem>()
                .FirstOrDefault(m => m.Header as string == segment && m.Items.Count > 0);

            if (existing == null)
            {
                existing = new Avalonia.Controls.MenuItem
                {
                    Header = segment,
                };
                parent.Items.Add(existing);
            }

            parent = existing;
        }

        return (Avalonia.Controls.MenuItem)parent;
    }

    /// <summary>将 EditorMenuItem 转换为 Avalonia MenuItem。</summary>
    private static Avalonia.Controls.MenuItem ConvertToAvaloniaMenuItem(EditorMenuItem item)
    {
        var menuItem = new Avalonia.Controls.MenuItem
        {
            Header = item.Path,
        };

        // 图标（emoji 或文本）
        if (!string.IsNullOrEmpty(item.Icon))
        {
            menuItem.Icon = new TextBlock
            {
                Text = item.Icon,
                FontSize = 14,
                Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            };
        }

        // CanExecute
        if (item.Command?.CanExecute != null)
        {
            menuItem.IsEnabled = item.Command.CanExecute();
        }

        // IsChecked
        if (item.Command?.IsChecked != null)
        {
            menuItem.IsChecked = item.Command.IsChecked();
        }

        // 点击执行命令
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
                    Console.Error.WriteLine($"[AvaloniaContextMenu] 命令执行失败 '{command.Id}': {ex.Message}");
                }
            };
        }

        return menuItem;
    }

}
