namespace Neverness.Runtime.Audio.Native;

/// <summary>
/// Native 音频播放器——持有 C++ AudioPlayer 的 opaque handle，
/// 委托所有操作给 NNAudio_* C ABI。
/// </summary>
public sealed class NativeAudioPlayer : IAudioPlayer
{
    private ulong _handle;
    private bool _disposed;
    private double _duration;

    internal NativeAudioPlayer()
    {
        _handle = AudioNative.NNAudio_CreatePlayer();
        if (_handle == 0)
            throw new InvalidOperationException("NNAudio_CreatePlayer 返回 0，Native 音频系统初始化失败。");
    }

    /// <inheritdoc />
    public bool Load(string filePath)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);

        if (!AudioNative.NNAudio_LoadFile(_handle, filePath))
            return false;

        _duration = AudioNative.NNAudio_GetDuration(_handle);
        return true;
    }

    /// <inheritdoc />
    public void Play()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        AudioNative.NNAudio_Play(_handle);
    }

    /// <inheritdoc />
    public void Pause()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        AudioNative.NNAudio_Pause(_handle);
    }

    /// <inheritdoc />
    public void Resume()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        AudioNative.NNAudio_Resume(_handle);
    }

    /// <inheritdoc />
    public void Stop()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        AudioNative.NNAudio_Stop(_handle);
    }

    /// <inheritdoc />
    public void Seek(double seconds)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        AudioNative.NNAudio_Seek(_handle, seconds);
    }

    /// <inheritdoc />
    public bool IsPlaying
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            return AudioNative.NNAudio_IsPlaying(_handle);
        }
    }

    /// <inheritdoc />
    public double PlaybackTime
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            return AudioNative.NNAudio_GetPlaybackTime(_handle);
        }
    }

    /// <inheritdoc />
    public double Duration => _duration;

    /// <inheritdoc />
    public float Volume
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            return AudioNative.NNAudio_GetVolume(_handle);
        }
        set
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            AudioNative.NNAudio_SetVolume(_handle, Math.Clamp(value, 0f, 1f));
        }
    }

    /// <inheritdoc />
    public bool Loop
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            return AudioNative.NNAudio_IsLooping(_handle);
        }
        set
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            AudioNative.NNAudio_SetLoop(_handle, value);
        }
    }

    /// <inheritdoc />
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        if (_handle != 0)
        {
            AudioNative.NNAudio_DestroyPlayer(_handle);
            _handle = 0;
        }
    }
}
