using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.ImGuiFrontend.Inspectors;

/// <summary>
/// 精灵渲染组件 Inspector——绘制颜色、UV、混合模式、排序等字段。
/// 排序在 Transform(0) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(50)]
public sealed class SpriteRendererInspector
    : ComponentTypeInspector<SpriteRendererComponent>
{
    /// <inheritdoc />
    public override int Order => 50;

    /// <inheritdoc />
    protected override bool DrawFields(ref SpriteRendererComponent data)
    {
        bool modified = false;

        // ── Color (RGBA) ──
        var color = data.Color;
        ImGui.Text("Color");
        ImGui.SameLine(100f);
        if (ImGui.ColorEdit4("##Color", ref color, ImGuiColorEditFlags.AlphaBar))
        {
            data.Color = color;
            modified = true;
        }

        // ── UV Rect ──
        var uv = data.UvRect;
        ImGui.Text("UV Rect");
        ImGui.SameLine(100f);
        ImGui.PushItemWidth(60f);
        modified |= ImGui.DragFloat("##u0", ref uv.X, 0.01f);
        ImGui.SameLine();
        modified |= ImGui.DragFloat("##v0", ref uv.Y, 0.01f);
        ImGui.SameLine();
        modified |= ImGui.DragFloat("##u1", ref uv.Z, 0.01f);
        ImGui.SameLine();
        modified |= ImGui.DragFloat("##v1", ref uv.W, 0.01f);
        ImGui.PopItemWidth();
        if (modified)
        {
            data.UvRect = uv;
        }

        // ── Layer ──
        int layer = (int)data.Layer;
        ImGui.Text("Layer");
        ImGui.SameLine(100f);
        if (ImGui.InputInt("##Layer", ref layer))
        {
            data.Layer = (uint)Math.Max(0, layer);
            modified = true;
        }

        // ── Sort Order ──
        int sortOrder = (int)data.SortOrder;
        ImGui.Text("Sort Order");
        ImGui.SameLine(100f);
        if (ImGui.InputInt("##SortOrder", ref sortOrder))
        {
            data.SortOrder = (uint)Math.Max(0, sortOrder);
            modified = true;
        }

        // ── Blend Mode ──
        int blend = (int)data.Blend;
        string[] blendNames = ["Alpha", "Additive", "Multiply", "Opaque", "Premultiplied"];
        ImGui.Text("Blend");
        ImGui.SameLine(100f);
        if (ImGui.Combo("##Blend", ref blend, blendNames, blendNames.Length))
        {
            data.Blend = (BlendMode)blend;
            modified = true;
        }

        // ── Flags ──
        modified |= DrawSpriteFlags(ref data);

        return modified;
    }

    /// <summary>绘制精灵标志位复选框（Visible / FlipX / FlipY）。</summary>
    private static bool DrawSpriteFlags(ref SpriteRendererComponent data)
    {
        bool modified = false;
        var flags = data.Flags;

        bool visible = flags.HasFlag(SpriteFlags.Visible);
        if (ImGui.Checkbox("Visible", ref visible))
        {
            flags = visible ? flags | SpriteFlags.Visible
                            : flags & ~SpriteFlags.Visible;
            modified = true;
        }

        bool flipX = flags.HasFlag(SpriteFlags.FlipX);
        ImGui.SameLine();
        if (ImGui.Checkbox("Flip X", ref flipX))
        {
            flags = flipX ? flags | SpriteFlags.FlipX
                          : flags & ~SpriteFlags.FlipX;
            modified = true;
        }

        bool flipY = flags.HasFlag(SpriteFlags.FlipY);
        ImGui.SameLine();
        if (ImGui.Checkbox("Flip Y", ref flipY))
        {
            flags = flipY ? flags | SpriteFlags.FlipY
                          : flags & ~SpriteFlags.FlipY;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }
}
