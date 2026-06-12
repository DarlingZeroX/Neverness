using Avalonia.Controls;
using Avalonia.Media;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.ContextMenus;

/// <summary>
/// Avalonia 上下文菜单渲染器——从 EditorMenuRegistry 读取菜单定义，渲染为 Avalonia ContextMenu。
///
/// 与 ImGui 的 ImGuiContextMenuRenderer 对应。
/// 实现 IContextMenuRenderer 接口，使 Core 模块不依赖具体 UI 框架。
/// </summary>
public class AvaloniaContextMenuRenderer : IContextMenuRenderer
{
    // 当前活跃的上下文菜单
    private ContextMenu? _activeMenu;
    private string? _activeContextId;

    /// <summary>开始窗口级弹出菜单（右键空白区域）。</summary>
    public bool BeginWindowPopup(string contextId)
    {
        // 在 Avalonia 中，ContextMenu 在显示时才创建
        // 此处标记开始，后续 RenderPopupContent 添加内容
        _activeContextId = contextId;
        _activeMenu = new ContextMenu
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };
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
        // 菜单在显示后自动管理生命周期
        _activeMenu = null;
        _activeContextId = null;
    }

    /// <summary>在弹出菜单内渲染所有已注册的内容。</summary>
    public void RenderPopupContent(string contextId, ContextMenuManager manager)
    {
        if (_activeMenu == null) return;

        // 从 ContextMenuManager 获取已注册的菜单项
        // TODO: 实现菜单注册表读取
        // 当前创建示例菜单项

        var deleteItem = new MenuItem
        {
            Header = "Delete",
            Foreground = new SolidColorBrush(Color.Parse("#FFF44336")),
        };
        deleteItem.Click += (_, _) => ExecuteCommand("edit.delete");
        _activeMenu.Items.Add(deleteItem);

        var renameItem = new MenuItem
        {
            Header = "Rename",
        };
        renameItem.Click += (_, _) => ExecuteCommand("edit.rename");
        _activeMenu.Items.Add(renameItem);

        var duplicateItem = new MenuItem
        {
            Header = "Duplicate",
        };
        duplicateItem.Click += (_, _) => ExecuteCommand("edit.duplicate");
        _activeMenu.Items.Add(duplicateItem);
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
    /// 在指定控件上显示上下文菜单。
    /// </summary>
    public void ShowOnControl(Control target, string contextId, ContextMenuManager manager)
    {
        RenderWindowContextMenu(contextId, manager);
        _activeMenu?.Open(target);
    }

    /// <summary>执行命令。</summary>
    private void ExecuteCommand(string commandId)
    {
        Console.WriteLine($"[AvaloniaContextMenu] 执行命令: {commandId}");
        // TODO: 通过 EditorCommandRegistry 执行命令
    }
}
