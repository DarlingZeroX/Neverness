using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.ImGuiFrontend.Menu;

/// <summary>
/// ImGui 上下文菜单渲染器——实现 IContextMenuRenderer 接口。
/// 从 ContextMenuManager 拆分出的 ImGui 渲染逻辑。
/// </summary>
public sealed class ImGuiContextMenuRenderer : IContextMenuRenderer
{
    /// <summary>开始窗口级弹出菜单（右键空白区域）。</summary>
    public bool BeginWindowPopup(string contextId) =>
        ImGui.BeginPopupContextWindow(contextId);

    /// <summary>开始项目级弹出菜单（右键某个项目）。</summary>
    public bool BeginItemPopup(string contextId) =>
        ImGui.BeginPopupContextItem(contextId);

    /// <summary>结束弹出菜单。</summary>
    public void EndPopup() => ImGui.EndPopup();

    /// <summary>在弹出菜单内渲染所有已注册的内容。</summary>
    public void RenderPopupContent(string contextId, ContextMenuManager manager)
    {
        // 1. 贡献者注册的静态项（已构建）
        manager.EnsureContributors();

        // 2. 回调式动态项
        var callbacks = manager.GetCallbacks(contextId);
        if (callbacks != null)
        {
            foreach (var cb in callbacks)
            {
                var builder = new ContextMenuBuilder();
                cb(builder);
                RenderItems(builder.Build());
            }
        }

        // 3. 静态注册的项
        var items = manager.GetStaticItems(contextId);
        if (items != null)
            RenderItems(items);
    }

    /// <summary>完整渲染窗口级上下文菜单（Begin → 内容 → End）。</summary>
    public void RenderWindowContextMenu(string contextId, ContextMenuManager manager)
    {
        manager.EnsureContributors();

        if (ImGui.IsWindowHovered() && !ImGui.IsAnyItemHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right))
            ImGui.OpenPopup(contextId);

        if (!ImGui.BeginPopup(contextId))
            return;
        RenderPopupContent(contextId, manager);
        ImGui.EndPopup();
    }

    /// <summary>完整渲染项目级上下文菜单（Begin → 内容 → End）。</summary>
    public void RenderItemContextMenu(string contextId, ContextMenuManager manager)
    {
        manager.EnsureContributors();
        if (!ImGui.BeginPopupContextItem(contextId))
            return;
        RenderPopupContent(contextId, manager);
        ImGui.EndPopup();
    }

    /// <summary>渲染菜单项列表。</summary>
    private static void RenderItems(IReadOnlyList<EditorMenuItem> items)
    {
        foreach (var item in items)
        {
            if (item.IsSeparator)
            {
                ImGui.Separator();
                continue;
            }

            var label = string.IsNullOrEmpty(item.Icon)
                ? item.Command?.DisplayName ?? ""
                : item.Icon + " " + (item.Command?.DisplayName ?? "");

            bool enabled = item.Command?.CanExecute?.Invoke() ?? true;
            var selected = item.Command?.IsChecked?.Invoke() ?? false;
            var hasCheck = item.Command?.IsChecked != null;

            if (hasCheck)
            {
                if (ImGui.MenuItem(label, string.IsNullOrEmpty(item.Shortcut) ? null : item.Shortcut, selected, enabled))
                {
                    item.Command?.Execute(default);
                    ImGui.CloseCurrentPopup();
                }
            }
            else
            {
                if (ImGui.MenuItem(label, string.IsNullOrEmpty(item.Shortcut) ? null : item.Shortcut, false, enabled))
                {
                    item.Command?.Execute(default);
                    ImGui.CloseCurrentPopup();
                }
            }
        }
    }
}
