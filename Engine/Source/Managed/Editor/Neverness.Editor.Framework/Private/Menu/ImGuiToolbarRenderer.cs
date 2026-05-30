using System.Numerics;
using Hexa.NET.ImGui;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// ImGui 工具栏渲染器——渲染工具栏按钮。
/// </summary>
public static class ImGuiToolbarRenderer
{
    private static readonly List<ToolbarCommand> s_commands = [];

    /// <summary>注册工具栏按钮。</summary>
    public static void Register(ToolbarCommand cmd) => s_commands.Add(cmd);

    /// <summary>渲染工具栏。</summary>
    public static void Render()
    {
        ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(4, 4));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(2, 2));

        foreach (var cmd in s_commands.OrderBy(c => c.SortOrder))
        {
            var registry = MenuRegistryImp.Instance;
            var editorCmd = registry.FindCommand(cmd.CommandId);
            bool enabled = editorCmd?.CanExecute?.Invoke() ?? true;

            // 禁用状态（使用 PushItemFlag 模拟 DisabledScope）
            if (!enabled)
            {
                ImGui.PushStyleVar(ImGuiStyleVar.Alpha, 0.5f);
            }

            if (ImGui.Button(cmd.Icon + "##toolbar_" + cmd.Id))
            {
                if (enabled)
                {
                    registry.ExecuteCommand(cmd.CommandId);
                }
            }

            if (!string.IsNullOrEmpty(cmd.Tooltip) && ImGui.IsItemHovered(ImGuiHoveredFlags.DelayShort))
            {
                ImGui.SetTooltip(cmd.Tooltip);
            }

            if (!enabled)
            {
                ImGui.PopStyleVar();
            }

            ImGui.SameLine();
        }

        ImGui.PopStyleVar(2);
    }
}
