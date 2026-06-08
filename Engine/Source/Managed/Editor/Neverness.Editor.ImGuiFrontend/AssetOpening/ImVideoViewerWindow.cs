using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.ImGuiEx;
using Neverness.Editor.Media;

namespace Neverness.Editor.ImGuiFrontend.AssetOpening;

/// <summary>
/// 视频查看器窗口——显示首帧预览和视频元信息。
/// 支持多实例（可同时打开多个视频资产）。
///
/// TODO: 完整视频播放需要 MediaRuntimeAPI 编译后实现。
/// </summary>
public sealed class ImVideoViewerWindow : ImWindow
{
    private ulong m_TextureHandle;
    private Vector2 m_TextureSize;

    /// <summary>资产名称（窗口标题用）。</summary>
    public string? AssetName { get; set; }

    /// <summary>资产 GUID 十六进制（用于 AssetEditorManager 映射）。</summary>
    public string? AssetGuidHex { get; set; }

    /// <summary>视频元信息。</summary>
    public VideoMetaInfo MetaInfo { get; set; }

    /// <summary>播放状态（未来使用）。</summary>
    private bool m_IsPlaying;
    private float m_Volume = 1f;

    public ImVideoViewerWindow() : base("Video Viewer")
    {
        BehaviorFlags = ImWindowBehaviorFlags.MultiInstance | ImWindowBehaviorFlags.Default;
        Flags = ImGuiWindowFlags.MenuBar;
    }

    /// <summary>设置首帧预览纹理。</summary>
    public void SetThumbnail(ulong textureHandle, Vector2 size)
    {
        m_TextureHandle = textureHandle;
        m_TextureSize = size;
        UpdateTitle();
    }

    private void UpdateTitle()
    {
        if (AssetName != null)
            Title = $"{AssetName} - Video";
        else if (m_TextureSize.X > 0)
            Title = $"Video Viewer - {(int)m_TextureSize.X}x{(int)m_TextureSize.Y}";
        else
            Title = "Video Viewer";
    }

    protected override bool HasMenuBar() => true;

    protected override void OnRenderMenuBar()
    {
        ImGui.TextDisabled(AssetName ?? "Unknown");
        ImGui.SameLine(ImGui.GetWindowWidth() - 280);
        ImGui.TextDisabled(
            $"{(int)MetaInfo.Width}x{(int)MetaInfo.Height} | " +
            $"{MetaInfo.FPS:F2}fps | " +
            $"{MetaInfo.Duration:F1}s");
    }

    protected override void OnRender()
    {
        var available = ImGui.GetContentRegionAvail();

        // ── 首帧预览 ──
        if (m_TextureHandle != 0 && m_TextureSize.X > 0)
        {
            float scaleX = available.X / m_TextureSize.X;
            float scaleY = (available.Y - 120f) / m_TextureSize.Y;
            float scale = Math.Min(Math.Min(scaleX, scaleY), 1f);
            var displaySize = m_TextureSize * scale;

            var cursor = ImGui.GetCursorScreenPos();
            var drawList = ImGui.GetWindowDrawList();

            // 黑色背景
            drawList.AddRectFilled(cursor, cursor + displaySize, 0xFF000000);

            ImGui.SetCursorScreenPos(cursor);
            unsafe
            {
                ImGui.Image(new ImTextureRef(null, m_TextureHandle), displaySize);
            }
        }
        else
        {
            ImGui.TextDisabled("首帧预览不可用（需要导入后生成）");
        }

        ImGui.Separator();

        // ── 元信息 ──
        ImGui.Text($"Resolution: {(int)MetaInfo.Width} x {(int)MetaInfo.Height}");
        ImGui.Text($"FPS: {MetaInfo.FPS:F2} ({MetaInfo.FpsNum}/{MetaInfo.FpsDen})");
        ImGui.Text($"Duration: {MetaInfo.Duration:F2} s");
        ImGui.Text($"Frame Count: {MetaInfo.FrameCount:N0}");
        ImGui.Text($"Codec: {MetaInfo.CodecName}");
        if (MetaInfo.HasAudio)
            ImGui.Text($"Audio: {MetaInfo.AudioSampleRate}Hz, {MetaInfo.AudioChannels}ch");

        // ── 播放控制（桩 UI，功能待 MediaRuntimeAPI）──
        ImGui.Separator();

        ImGui.BeginDisabled();
        if (ImGui.Button(m_IsPlaying ? "Pause" : "Play"))
            m_IsPlaying = !m_IsPlaying;
        ImGui.SameLine();
        ImGui.SliderFloat("##Volume", ref m_Volume, 0f, 1f);
        ImGui.EndDisabled();

        ImGui.SameLine();
        ImGui.TextDisabled("(播放控制待 MediaRuntimeAPI)");
    }
}
