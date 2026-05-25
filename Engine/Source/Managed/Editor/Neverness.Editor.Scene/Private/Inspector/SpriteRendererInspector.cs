using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.Scene.Private.Inspector;

/// <summary>
/// 精灵渲染组件 Inspector——绘制纹理、颜色、UV、混合模式、排序等字段。
/// 排序在 Transform(0) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(50)]
public sealed class SpriteRendererInspector
    : ComponentTypeInspector<NNSpriteRendererComponentData>
{
    /// <inheritdoc />
    public override int Order => 50;

    /// <inheritdoc />
    protected override bool DrawFields(ref NNSpriteRendererComponentData data)
    {
        bool modified = false;

        // ── Texture Asset ──
        ImGui.Text("Texture");
        ImGui.SameLine(100f);
        // TODO: Asset Picker（后续 Phase 实现）
        ulong textureHash = data.TextureAsset;
        ImGui.Text($"0x{textureHash:X16}");

        // ── Material Asset ──
        ImGui.Text("Material");
        ImGui.SameLine(100f);
        ulong materialHash = data.MaterialAsset;
        ImGui.Text($"0x{materialHash:X16}");

        // ── Color (RGBA) ──
        var color = new Vector4(data.ColorR, data.ColorG, data.ColorB, data.ColorA);
        ImGui.Text("Color");
        ImGui.SameLine(100f);
        if (ImGui.ColorEdit4("##Color", ref color, ImGuiColorEditFlags.AlphaBar))
        {
            data.ColorR = color.X;
            data.ColorG = color.Y;
            data.ColorB = color.Z;
            data.ColorA = color.W;
            modified = true;
        }

        // ── UV Rect ──
        var uv = new Vector4(data.UvU0, data.UvV0, data.UvU1, data.UvV1);
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
            data.UvU0 = uv.X;
            data.UvV0 = uv.Y;
            data.UvU1 = uv.Z;
            data.UvV1 = uv.W;
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
        int blend = (int)data.BlendMode;
        string[] blendNames = ["Alpha", "Additive", "Multiply", "Opaque", "Premultiplied"];
        ImGui.Text("Blend");
        ImGui.SameLine(100f);
        if (ImGui.Combo("##Blend", ref blend, blendNames, blendNames.Length))
        {
            data.BlendMode = (uint)blend;
            modified = true;
        }

        // ── Flags ──
        modified |= DrawSpriteFlags(ref data);

        return modified;
    }

    /// <summary>绘制精灵标志位复选框（Visible / FlipX / FlipY）。</summary>
    private static bool DrawSpriteFlags(ref NNSpriteRendererComponentData data)
    {
        bool modified = false;
        uint flags = data.Flags;

        bool visible = (flags & (uint)NNSpriteFlags.Visible) != 0;
        if (ImGui.Checkbox("Visible", ref visible))
        {
            flags = visible ? flags | (uint)NNSpriteFlags.Visible
                            : flags & ~(uint)NNSpriteFlags.Visible;
            modified = true;
        }

        bool flipX = (flags & (uint)NNSpriteFlags.FlipX) != 0;
        ImGui.SameLine();
        if (ImGui.Checkbox("Flip X", ref flipX))
        {
            flags = flipX ? flags | (uint)NNSpriteFlags.FlipX
                          : flags & ~(uint)NNSpriteFlags.FlipX;
            modified = true;
        }

        bool flipY = (flags & (uint)NNSpriteFlags.FlipY) != 0;
        ImGui.SameLine();
        if (ImGui.Checkbox("Flip Y", ref flipY))
        {
            flags = flipY ? flags | (uint)NNSpriteFlags.FlipY
                          : flags & ~(uint)NNSpriteFlags.FlipY;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }
}
