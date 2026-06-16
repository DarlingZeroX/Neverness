using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Media;

/// <summary>
/// 音频源组件 Inspector——编辑音量、音调、3D 空间音频、播放标志位。
/// 排序在 SpriteRenderer(50) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(60)]
public sealed class AudioSourceInspector
    : ComponentTypeInspector<AudioSourceComponent>
{
    public override int Order => 60;

    protected override bool DrawFields(ref AudioSourceComponent data)
    {
        bool modified = false;

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
        bool spatial = data.Flags.HasFlag(AudioSourceFlags.Spatial);
        if (ImGui.Checkbox("Spatial Audio", ref spatial))
        {
            data.Flags = spatial
                ? data.Flags | AudioSourceFlags.Spatial
                : data.Flags & ~AudioSourceFlags.Spatial;
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
    private static bool DrawAudioFlags(ref AudioSourceComponent data)
    {
        bool modified = false;
        var flags = data.Flags;

        bool playOnAwake = flags.HasFlag(AudioSourceFlags.AutoPlay);
        if (ImGui.Checkbox("Play On Awake", ref playOnAwake))
        {
            flags = playOnAwake ? flags | AudioSourceFlags.AutoPlay
                                : flags & ~AudioSourceFlags.AutoPlay;
            modified = true;
        }

        ImGui.SameLine();
        bool loop = flags.HasFlag(AudioSourceFlags.Loop);
        if (ImGui.Checkbox("Loop", ref loop))
        {
            flags = loop ? flags | AudioSourceFlags.Loop
                         : flags & ~AudioSourceFlags.Loop;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }
}
