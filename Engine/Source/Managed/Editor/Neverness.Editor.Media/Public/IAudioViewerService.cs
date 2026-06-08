using Neverness.Runtime.Audio;

namespace Neverness.Editor.Media;

/// <summary>
/// 音频查看器服务接口——由 ImGuiFrontend 模块实现。
/// Media 模块的 AssetOpener 通过此接口创建查看器窗口。
/// </summary>
public interface IAudioViewerService
{
    /// <summary>打开音频查看器窗口。</summary>
    void OpenViewer(AudioViewerOptions options);
}

/// <summary>
/// 音频查看器选项。
/// </summary>
public class AudioViewerOptions
{
    public string? AssetName { get; set; }
    public string? AssetGuidHex { get; set; }
    public AudioMetaInfo MetaInfo { get; set; }
    public IAudioPlayer? Player { get; set; }
    public ulong WaveformHandle { get; set; }
    public System.Numerics.Vector2 WaveformSize { get; set; }
}
