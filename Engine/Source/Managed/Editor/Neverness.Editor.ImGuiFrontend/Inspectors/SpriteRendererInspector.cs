using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.ImGuiFrontend.Inspectors;

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

    // ── 纹理缓存：避免每帧重复加载 ──
    private GUID _cachedTextureGuid;
    private ulong _cachedImGuiTexHandle;

    /// <inheritdoc />
    protected override bool DrawFields(ref NNSpriteRendererComponentData data)
    {
        bool modified = false;

        // ── Texture Asset（Drop Zone）──
        ImGui.Text("Texture");
        ImGui.SameLine(100f);

        float dropWidth = Math.Max(ImGui.GetContentRegionAvail().X, 128f);
        float dropHeight = 128f;
        ImGui.InvisibleButton("##TextureDropZone", new Vector2(dropWidth, dropHeight));
        var dropMin = ImGui.GetItemRectMin();
        ImDrawListPtr drawList = ImGui.GetWindowDrawList();

        GUID textureGuid = GUID.FromNative(data.TextureAsset);
        ulong imTexHandle = 0;

        if (!textureGuid.IsZero)
        {
            // 仅当 GUID 变化时才重新加载纹理，避免每帧刷日志
            if (textureGuid != _cachedTextureGuid)
            {
                _cachedTextureGuid = textureGuid;
                _cachedImGuiTexHandle = 0;

                var texHandle = AssetHandleExtensions.LoadSync(textureGuid, 1);
                ulong cacheKey = 0;
                if (!texHandle.IsZero)
                    cacheKey = TextureInterop.LoadTextureFromBlob(new NNAssetHandle(texHandle.Value), data.TextureAsset);
                if (cacheKey != 0)
                    _cachedImGuiTexHandle = TextureInterop.GetImGuiTextureHandle(cacheKey);
            }
            imTexHandle = _cachedImGuiTexHandle;

            if (imTexHandle != 0)
            {
                float previewSize = Math.Min(dropWidth, dropHeight);
                unsafe
                {
                    drawList.AddImage(
                        new ImTextureRef(null, imTexHandle),
                        dropMin,
                        dropMin + new Vector2(previewSize, previewSize));
                }
            }
            else
            {
                drawList.AddText(dropMin, ImGui.GetColorU32(ImGuiCol.Text),
                    $"0x{textureGuid.Low:X16} (未加载)");
            }
        }
        else
        {
            // GUID 为零时清除缓存
            if (!_cachedTextureGuid.IsZero)
            {
                _cachedTextureGuid = default;
                _cachedImGuiTexHandle = 0;
            }
            drawList.AddText(dropMin, ImGui.GetColorU32(ImGuiCol.TextDisabled),
                "None (drop texture here)");
        }

        // ── Drop Target：接收纹理资产拖拽 ──
        using (var target = AssetDragDrop.BeginDragDropTarget())
        {
            if (target.IsActive)
            {
                if (AssetDragDrop.TryAcceptDragDrop(AssetDragDrop.TypeIdTexture2D, out var droppedGuid, out _))
                {
                    // 存储完整 128-bit GUID（持久化标识），Renderer 在 Collect 时懒解析为 GL ID
                    data.TextureAsset = droppedGuid.ToNative();
                    modified = true;
                    Console.WriteLine($"[Inspector] Drop: GUID=({droppedGuid.High},{droppedGuid.Low}) → TextureAsset");
                }
            }
        }

        // ── 右键清除纹理 ──
        if (ImGui.BeginPopupContextItem("##TextureCtx"))
        {
            if (data.TextureAsset.Low != 0 && ImGui.MenuItem("Clear Texture"))
            {
                data.TextureAsset = default;
                modified = true;
            }
            ImGui.EndPopup();
        }

        ImGui.Text($"textureGuid: 0x{textureGuid.Low:X16}");
        ImGui.Text($"imTexHandle: {imTexHandle}");

        // ── Material Asset ──
        ImGui.Text("Material");
        ImGui.SameLine(100f);
        GUID materialGuid = GUID.FromNative(data.MaterialAsset);
        ImGui.Text(materialGuid.IsZero ? "Default" : $"0x{materialGuid.Low:X16}");

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
