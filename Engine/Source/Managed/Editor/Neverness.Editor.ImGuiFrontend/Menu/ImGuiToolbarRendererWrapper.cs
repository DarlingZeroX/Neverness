using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.ImGuiFrontend.Menu;

/// <summary>
/// ImGui 工具栏渲染器——实现 IToolbarRenderer 接口。
/// 从 ToolbarRegistry 读取按钮列表，使用 ImGui 渲染。
///
/// 从 Framework/Private/Menu/ImGuiToolbarRenderer.cs 搬迁而来。
/// </summary>
public sealed class ImGuiToolbarRendererWrapper : IToolbarRenderer
{
    private readonly ToolbarRegistry _registry = ToolbarRegistry.Instance;

    /// <summary>注册工具栏按钮（委托给 ToolbarRegistry）。</summary>
    public void Register(ToolbarCommand cmd) => _registry.Register(cmd);

    /// <summary>渲染工具栏。</summary>
    public void Render()
    {
        ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(4, 4));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(2, 2));

        var commands = _registry.GetCommands();
        var menuRegistry = MenuRegistryImp.Instance;

        foreach (var cmd in commands)
        {
            var editorCmd = menuRegistry.FindCommand(cmd.CommandId);
            bool enabled = editorCmd?.CanExecute?.Invoke() ?? true;

            // 禁用状态
            if (!enabled)
            {
                ImGui.PushStyleVar(ImGuiStyleVar.Alpha, 0.5f);
            }

            if (ImGui.Button(cmd.Icon + "##toolbar_" + cmd.Id))
            {
                if (enabled)
                {
                    menuRegistry.ExecuteCommand(cmd.CommandId);
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
