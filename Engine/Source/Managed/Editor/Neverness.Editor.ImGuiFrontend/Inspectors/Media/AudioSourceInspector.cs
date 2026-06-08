using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Media;

/// <summary>
/// 音频源组件 Inspector——编辑 AudioClip 资产引用、音量、音调、3D 空间音频、播放标志位。
/// 排序在 SpriteRenderer(50) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(60)]
public sealed class AudioSourceInspector
    : ComponentTypeInspector<NNAudioSourceComponentData>
{
    public override int Order => 60;

    protected override bool DrawFields(ref NNAudioSourceComponentData data)
    {
        bool modified = false;

        // ── AudioClip 资产引用 ──
        modified |= DrawGuidField("Audio Clip", ref data.AudioClipAsset, AssetDragDrop.TypeIdAudioClip);

        // ── Volume ──
        ImGui.Text("Volume");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.SliderFloat("##Volume", ref data.Volume, 0f, 1f);
        ImGui.PopItemWidth();

        // ── Pitch ──
        ImGui.Text("Pitch");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.SliderFloat("##Pitch", ref data.Pitch, 0.5f, 2f);
        ImGui.PopItemWidth();

        // ── Spatial Audio ──
        bool spatial = (data.Flags & (uint)NNAudioSourceFlags.Spatial) != 0;
        if (ImGui.Checkbox("Spatial Audio", ref spatial))
        {
            data.Flags = spatial
                ? data.Flags | (uint)NNAudioSourceFlags.Spatial
                : data.Flags & ~(uint)NNAudioSourceFlags.Spatial;
            modified = true;
        }

        if (spatial)
        {
            ImGui.Text("Min Dist");
            ImGui.SameLine(120f);
            ImGui.PushItemWidth(200f);
            modified |= ImGui.DragFloat("##MinDist", ref data.MinDistance, 0.1f, 0f, 1000f);
            ImGui.PopItemWidth();

            ImGui.Text("Max Dist");
            ImGui.SameLine(120f);
            ImGui.PushItemWidth(200f);
            modified |= ImGui.DragFloat("##MaxDist", ref data.MaxDistance, 0.1f, 0f, 10000f);
            ImGui.PopItemWidth();
        }

        // ── 标志位 ──
        modified |= DrawAudioFlags(ref data);

        return modified;
    }

    /// <summary>绘制音频源标志位复选框。</summary>
    private static bool DrawAudioFlags(ref NNAudioSourceComponentData data)
    {
        bool modified = false;
        uint flags = data.Flags;

        bool playOnAwake = (flags & (uint)NNAudioSourceFlags.PlayOnAwake) != 0;
        if (ImGui.Checkbox("Play On Awake", ref playOnAwake))
        {
            flags = playOnAwake ? flags | (uint)NNAudioSourceFlags.PlayOnAwake
                                : flags & ~(uint)NNAudioSourceFlags.PlayOnAwake;
            modified = true;
        }

        ImGui.SameLine();
        bool loop = (flags & (uint)NNAudioSourceFlags.Loop) != 0;
        if (ImGui.Checkbox("Loop", ref loop))
        {
            flags = loop ? flags | (uint)NNAudioSourceFlags.Loop
                         : flags & ~(uint)NNAudioSourceFlags.Loop;
            modified = true;
        }

        ImGui.SameLine();
        bool mute = (flags & (uint)NNAudioSourceFlags.Mute) != 0;
        if (ImGui.Checkbox("Mute", ref mute))
        {
            flags = mute ? flags | (uint)NNAudioSourceFlags.Mute
                         : flags & ~(uint)NNAudioSourceFlags.Mute;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }

    /// <summary>绘制 GUID 资产引用字段（文本显示 + 拖放接收 + 右键清除）。</summary>
    private static bool DrawGuidField(string label, ref NNGuid guid, ulong expectedTypeId)
    {
        bool modified = false;

        ImGui.Text(label);
        ImGui.SameLine(120f);

        var typedGuid = GUID.FromNative(guid);
        string guidText = typedGuid.IsZero ? "None" : typedGuid.ToHexString();

        ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.2f, 0.2f, 0.2f, 1f));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.3f, 0.3f, 0.3f, 1f));
        ImGui.Button($"{typedGuid.ToString()}##Btn", new Vector2(200f, 0));
        ImGui.PopStyleColor(2);

        // 右键清除
        if (ImGui.BeginPopupContextItem($"##{label}Ctx"))
        {
            if (!typedGuid.IsZero && ImGui.MenuItem("Clear"))
            {
                guid = default;
                modified = true;
            }
            ImGui.EndPopup();
        }

        // 拖放接收
        using (var target = AssetDragDrop.BeginDragDropTarget())
        {
            if (target.IsActive)
            {
                if (AssetDragDrop.TryAcceptDragDrop(expectedTypeId, out var droppedGuid, out _))
                {
                    guid = droppedGuid.ToNative();
                    modified = true;
                }
            }
        }

        // GUID 文本
        ImGui.SameLine();
        ImGui.TextDisabled(guidText);

        return modified;
    }
}
