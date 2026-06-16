using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Rmlui;

/// <summary>
/// RmlUI 文档组件 Inspector——编辑标志位、排序、视图目标。
/// 排序在 VideoPlayer(65) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(70)]
public sealed class RmlUIDocumentInspector
    : ComponentTypeInspector<RmlUIDocumentComponent>
{
    public override int Order => 70;

    protected override bool DrawFields(ref RmlUIDocumentComponent data)
    {
        bool modified = false;

        // ── SortOrder ──
        ImGui.Text("Sort Order");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.DragInt("##SortOrder", ref data.SortOrder, 1f, -1000, 1000);
        ImGui.PopItemWidth();

        // ── ViewTarget ──
        modified |= DrawViewTarget(ref data);

        // ── Flags ──
        modified |= DrawFlags(ref data);

        return modified;
    }

    /// <summary>绘制视图目标下拉框。</summary>
    private static bool DrawViewTarget(ref RmlUIDocumentComponent data)
    {
        int current = (int)data.ViewTarget;
        string[] items = ["Scene", "Game", "Both"];

        ImGui.Text("View Target");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        bool changed = ImGui.Combo("##ViewTarget", ref current, items, items.Length);
        ImGui.PopItemWidth();

        if (changed)
            data.ViewTarget = (RmlUIViewTarget)current;

        return changed;
    }

    /// <summary>绘制标志位复选框。</summary>
    private static bool DrawFlags(ref RmlUIDocumentComponent data)
    {
        bool modified = false;
        var flags = data.Flags;

        bool autoLoad = flags.HasFlag(RmlUIDocumentFlags.AutoLoad);
        if (ImGui.Checkbox("Auto Load", ref autoLoad))
        {
            flags = autoLoad ? flags | RmlUIDocumentFlags.AutoLoad
                             : flags & ~RmlUIDocumentFlags.AutoLoad;
            modified = true;
        }

        bool focusable = flags.HasFlag(RmlUIDocumentFlags.Focusable);
        ImGui.SameLine();
        if (ImGui.Checkbox("Focusable", ref focusable))
        {
            flags = focusable ? flags | RmlUIDocumentFlags.Focusable
                              : flags & ~RmlUIDocumentFlags.Focusable;
            modified = true;
        }

        bool receivesInput = flags.HasFlag(RmlUIDocumentFlags.ReceivesInput);
        ImGui.SameLine();
        if (ImGui.Checkbox("Receives Input", ref receivesInput))
        {
            flags = receivesInput ? flags | RmlUIDocumentFlags.ReceivesInput
                                  : flags & ~RmlUIDocumentFlags.ReceivesInput;
            modified = true;
        }

        if (modified)
            data.Flags = flags;

        return modified;
    }
}
