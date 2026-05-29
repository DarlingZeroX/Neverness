using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.ImGuiEx;
using Neverness.Runtime.Audio;

namespace Neverness.Editor.Media;

/// <summary>
/// 音频查看器窗口——显示元信息 + 播放控制。
/// 支持多实例（可同时打开多个音频资产）。
/// </summary>
public sealed class ImAudioViewerWindow : ImWindow
{
    private ulong m_TextureHandle;
    private Vector2 m_TextureSize;

    private IAudioPlayer? _player;
    private float _volume = 1f;
    private bool _loop;
    private bool _seeking;

    /// <summary>资产名称（窗口标题用）。</summary>
    public string? AssetName { get; set; }

    /// <summary>资产 GUID 十六进制（用于 AssetEditorManager 映射）。</summary>
    public string? AssetGuidHex { get; set; }

    /// <summary>音频元信息。</summary>
    public AudioMetaInfo MetaInfo { get; set; }

    public ImAudioViewerWindow() : base("Audio Viewer")
    {
        BehaviorFlags = ImWindowBehaviorFlags.MultiInstance | ImWindowBehaviorFlags.Default;
        Flags = ImGuiWindowFlags.MenuBar;
    }

    /// <summary>设置播放器（由 AudioAssetOpener 调用）。</summary>
    public void SetPlayer(IAudioPlayer player)
    {
        _player = player;
        _volume = player.Volume;
        _loop = player.Loop;
    }

    /// <summary>设置波形图纹理。</summary>
    public void SetWaveform(ulong textureHandle, Vector2 size)
    {
        m_TextureHandle = textureHandle;
        m_TextureSize = size;
        UpdateTitle();
    }

    private void UpdateTitle()
    {
        if (AssetName != null)
            Title = $"{AssetName} - Audio";
        else
            Title = "Audio Viewer";
    }

    protected override bool HasMenuBar() => true;

    protected override void OnRenderMenuBar()
    {
        ImGui.TextDisabled(AssetName ?? "Unknown");
        ImGui.SameLine(ImGui.GetWindowWidth() - 220);
        var dur = _player?.Duration ?? MetaInfo.Duration;
        ImGui.TextDisabled($"{MetaInfo.Format} | {dur:F1}s");
    }

    protected override void OnRender()
    {
        // ── 波形图 ──
        if (m_TextureHandle != 0 && m_TextureSize.X > 0)
        {
            var available = ImGui.GetContentRegionAvail();
            float scaleX = available.X / m_TextureSize.X;
            float displayHeight = Math.Min(m_TextureSize.Y * scaleX, 160f);
            float displayWidth = available.X;

            ImGui.Text("Waveform");
            unsafe
            {
                ImGui.Image(new ImTextureRef(null, m_TextureHandle),
                    new Vector2(displayWidth, displayHeight));
            }
        }

        // ── 元信息 ──
        ImGui.Separator();
        ImGui.Text($"Sample Rate: {MetaInfo.SampleRate} Hz");
        ImGui.Text($"Channels: {MetaInfo.Channels} ({(MetaInfo.Channels == 1 ? "Mono" : MetaInfo.Channels == 2 ? "Stereo" : $"{MetaInfo.Channels}ch")})");
        ImGui.Text($"Format: {MetaInfo.Format}");

        // ── 播放控制 ──
        if (_player != null)
        {
            DrawPlaybackControls();
        }
        else
        {
            ImGui.Separator();
            ImGui.TextDisabled("播放器不可用");
        }
    }

    private void DrawPlaybackControls()
    {
        ImGui.Separator();

        bool isPlaying = _player!.IsPlaying;
        double currentTime = _player.PlaybackTime;
        double duration = _player.Duration;

        // ── 播放按钮行 ──
        // Play/Pause
        if (isPlaying)
        {
            if (ImGui.Button("Pause"))
                _player.Pause();
        }
        else
        {
            if (ImGui.Button("Play"))
                _player.Resume();
        }

        ImGui.SameLine();

        // Stop
        if (ImGui.Button("Stop"))
            _player.Stop();

        ImGui.SameLine();

        // Loop
        if (ImGui.Checkbox("Loop", ref _loop))
        {
            _player.Loop = _loop;
        }

        ImGui.SameLine();

        // Volume
        ImGui.PushItemWidth(100f);
        if (ImGui.SliderFloat("##Vol", ref _volume, 0f, 1f))
        {
            _player.Volume = _volume;
        }
        ImGui.PopItemWidth();
        ImGui.SameLine();
        ImGui.TextDisabled("Vol");

        // ── 进度条 ──
        float progress = duration > 0 ? (float)(currentTime / duration) : 0f;

        ImGui.PushItemWidth(-1);
        if (ImGui.SliderFloat("##Progress", ref progress, 0f, 1f,
            $"{FormatTime(currentTime)} / {FormatTime(duration)}"))
        {
            _seeking = true;
        }
        ImGui.PopItemWidth();

        // 鼠标释放时执行 Seek
        if (_seeking && ImGui.IsItemDeactivatedAfterEdit())
        {
            _player.Seek(progress * duration);
            _seeking = false;
        }

        // 拖拽过程中实时 Seek
        if (_seeking && ImGui.IsItemActive())
        {
            _player.Seek(progress * duration);
        }
    }

    private static string FormatTime(double seconds)
    {
        if (seconds < 0) seconds = 0;
        int min = (int)(seconds / 60);
        double sec = seconds % 60;
        return $"{min:D2}:{sec:05.2f}";
    }

    public override void OnClose()
    {
        _player?.Dispose();
        _player = null;
    }
}

/// <summary>音频元信息（从源文件解析）。</summary>
public struct AudioMetaInfo
{
    public uint SampleRate;
    public uint Channels;
    public ulong SampleCount;
    public double Duration;
    public string Format;
}
