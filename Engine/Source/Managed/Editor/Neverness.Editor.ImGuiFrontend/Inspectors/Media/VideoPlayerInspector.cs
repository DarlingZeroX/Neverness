using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Media;

/// <summary>
/// 视频播放器组件 Inspector——编辑音量、播放标志位。
/// 排序在 AudioSource(60) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(65)]
public sealed class VideoPlayerInspector
    : ComponentTypeInspector<VideoPlayerComponent>
{
    public override int Order => 65;

    protected override bool DrawFields(ref VideoPlayerComponent data)
    {
        bool modified = false;

        // ── Volume ──
        ImGui.Text("Volume");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.SliderFloat("##Volume", ref data.Volume, 0f, 1f);
        ImGui.PopItemWidth();

        // ── 标志位 ──
        modified |= DrawVideoFlags(ref data);

        // ── 只读调试信息 ──
        ImGui.Separator();
        ImGui.TextDisabled($"RuntimePlayerId: {data.RuntimePlayerId}");
        ImGui.TextDisabled($"VideoTextureId: {data.VideoTextureId}");

        return modified;
    }

    /// <summary>绘制视频播放器标志位复选框。</summary>
    private static bool DrawVideoFlags(ref VideoPlayerComponent data)
    {
        bool modified = false;
        var flags = data.Flags;

        bool playOnAwake = flags.HasFlag(VideoPlayerFlags.AutoPlay);
        if (ImGui.Checkbox("Play On Awake", ref playOnAwake))
        {
            flags = playOnAwake ? flags | VideoPlayerFlags.AutoPlay
                                : flags & ~VideoPlayerFlags.AutoPlay;
            modified = true;
        }

        ImGui.SameLine();
        bool loop = flags.HasFlag(VideoPlayerFlags.Loop);
        if (ImGui.Checkbox("Loop", ref loop))
        {
            flags = loop ? flags | VideoPlayerFlags.Loop
                         : flags & ~VideoPlayerFlags.Loop;
            modified = true;
        }

        bool mute = flags.HasFlag(VideoPlayerFlags.Muted);
        if (ImGui.Checkbox("Mute", ref mute))
        {
            flags = mute ? flags | VideoPlayerFlags.Muted
                         : flags & ~VideoPlayerFlags.Muted;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }
}
