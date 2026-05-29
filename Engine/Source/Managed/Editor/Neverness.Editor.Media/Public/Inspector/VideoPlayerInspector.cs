using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.Inspector;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.Media;

/// <summary>
/// 视频播放器组件 Inspector——编辑 VideoClip 资产引用、音量、目标 Sprite、播放标志位。
/// 排序在 AudioSource(60) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(65)]
public sealed class VideoPlayerInspector
    : ComponentTypeInspector<NNVideoPlayerComponentData>
{
    public override int Order => 65;

    protected override bool DrawFields(ref NNVideoPlayerComponentData data)
    {
        bool modified = false;

        // ── VideoClip 资产引用 ──
        modified |= DrawGuidField("Video Clip", ref data.VideoClipAsset, AssetDragDrop.TypeIdVideoClip);

        // ── Volume ──
        ImGui.Text("Volume");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.SliderFloat("##Volume", ref data.Volume, 0f, 1f);
        ImGui.PopItemWidth();

        // ── Target Sprite ──
        modified |= DrawGuidField("Target Sprite", ref data.TargetSprite, 0);

        // ── 标志位 ──
        modified |= DrawVideoFlags(ref data);

        // ── 只读调试信息 ──
        ImGui.Separator();
        ImGui.TextDisabled($"RuntimePlayerId: {data.RuntimePlayerId}");
        ImGui.TextDisabled($"VideoTextureId: {data.VideoTextureId}");

        return modified;
    }

    /// <summary>绘制视频播放器标志位复选框。</summary>
    private static bool DrawVideoFlags(ref NNVideoPlayerComponentData data)
    {
        bool modified = false;
        uint flags = data.Flags;

        bool playOnAwake = (flags & (uint)NNVideoPlayerFlags.PlayOnAwake) != 0;
        if (ImGui.Checkbox("Play On Awake", ref playOnAwake))
        {
            flags = playOnAwake ? flags | (uint)NNVideoPlayerFlags.PlayOnAwake
                                : flags & ~(uint)NNVideoPlayerFlags.PlayOnAwake;
            modified = true;
        }

        ImGui.SameLine();
        bool loop = (flags & (uint)NNVideoPlayerFlags.Loop) != 0;
        if (ImGui.Checkbox("Loop", ref loop))
        {
            flags = loop ? flags | (uint)NNVideoPlayerFlags.Loop
                         : flags & ~(uint)NNVideoPlayerFlags.Loop;
            modified = true;
        }

        bool renderToSprite = (flags & (uint)NNVideoPlayerFlags.RenderToSprite) != 0;
        if (ImGui.Checkbox("Render To Sprite", ref renderToSprite))
        {
            flags = renderToSprite ? flags | (uint)NNVideoPlayerFlags.RenderToSprite
                                   : flags & ~(uint)NNVideoPlayerFlags.RenderToSprite;
            modified = true;
        }

        ImGui.SameLine();
        bool mute = (flags & (uint)NNVideoPlayerFlags.Mute) != 0;
        if (ImGui.Checkbox("Mute", ref mute))
        {
            flags = mute ? flags | (uint)NNVideoPlayerFlags.Mute
                         : flags & ~(uint)NNVideoPlayerFlags.Mute;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }

    /// <summary>绘制 GUID 资产引用字段。</summary>
    private static bool DrawGuidField(string label, ref NNGuid guid, ulong expectedTypeId)
    {
        bool modified = false;

        ImGui.Text(label);
        ImGui.SameLine(120f);

        var typedGuid = GUID.FromNative(guid);
        string guidText = typedGuid.IsZero ? "None" : typedGuid.ToHexString();

        ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.2f, 0.2f, 0.2f, 1f));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.3f, 0.3f, 0.3f, 1f));
        ImGui.Button($"##{label}Btn", new Vector2(200f, 0));
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
