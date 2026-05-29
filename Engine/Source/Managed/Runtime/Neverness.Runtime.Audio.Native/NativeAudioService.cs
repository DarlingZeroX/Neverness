namespace Neverness.Runtime.Audio.Native;

/// <summary>
/// Native 音频服务工厂——创建 NativeAudioPlayer 实例。
/// </summary>
public sealed class NativeAudioService : IAudioService
{
    /// <inheritdoc />
    public IAudioPlayer CreatePlayer()
    {
        return new NativeAudioPlayer();
    }
}
