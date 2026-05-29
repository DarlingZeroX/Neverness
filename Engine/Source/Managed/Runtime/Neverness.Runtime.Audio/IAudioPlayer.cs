namespace Neverness.Runtime.Audio;

/// <summary>
/// 音频播放器接口——Runtime 和 Editor 共用的音频播放抽象。
///
/// 实现方负责 SDL3/FFmpeg 等 Native backend，调用方只通过此接口操作。
/// </summary>
public interface IAudioPlayer : IDisposable
{
    /// <summary>加载音频文件（WAV/MP3/OGG/FLAC）。</summary>
    /// <returns>加载成功返回 true。</returns>
    bool Load(string filePath);

    /// <summary>从头播放。</summary>
    void Play();

    /// <summary>暂停播放。</summary>
    void Pause();

    /// <summary>恢复播放（从暂停位置继续）。</summary>
    void Resume();

    /// <summary>停止播放（释放 Native 资源）。</summary>
    void Stop();

    /// <summary>跳转到指定时间（秒）。</summary>
    void Seek(double seconds);

    /// <summary>是否正在播放。</summary>
    bool IsPlaying { get; }

    /// <summary>当前播放时间（秒）。</summary>
    double PlaybackTime { get; }

    /// <summary>音频总时长（秒）。</summary>
    double Duration { get; }

    /// <summary>音量 [0.0, 1.0]。</summary>
    float Volume { get; set; }

    /// <summary>是否循环播放。</summary>
    bool Loop { get; set; }
}
