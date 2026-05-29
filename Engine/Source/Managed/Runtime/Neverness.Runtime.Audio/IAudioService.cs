namespace Neverness.Runtime.Audio;

/// <summary>
/// 音频服务接口——创建 IAudioPlayer 实例的工厂。
///
/// Editor 和 Scene AudioComponent 都通过此接口获取播放器，
/// 不直接依赖具体的 Native 实现。
/// </summary>
public interface IAudioService
{
    /// <summary>创建一个新的音频播放器实例。</summary>
    /// <returns>播放器实例，使用完毕后需 Dispose。</returns>
    IAudioPlayer CreatePlayer();
}
