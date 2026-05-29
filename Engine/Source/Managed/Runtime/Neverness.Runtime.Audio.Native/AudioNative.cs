using System.Runtime.InteropServices;

namespace Neverness.Runtime.Audio.Native;

/// <summary>
/// 音频 Native API P/Invoke 声明。
///
/// 加载 NevernessRuntime-Media DLL，解析 NNAudio_* C ABI 函数。
/// 所有函数委托给 NN::Core::AudioPlayer（SDL3 + FAudioDecoder）。
/// </summary>
internal static unsafe partial class AudioNative
{
    private const string Lib = "NevernessRuntime-Media";

    // ── 玩家管理 ──

    [LibraryImport(Lib)]
    internal static partial ulong NNAudio_CreatePlayer();

    [LibraryImport(Lib)]
    internal static partial void NNAudio_DestroyPlayer(ulong handle);

    // ── 文件加载 ──

    [LibraryImport(Lib, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static partial bool NNAudio_LoadFile(ulong handle, string filePathUtf8);

    // ── 播放控制 ──

    [LibraryImport(Lib)]
    internal static partial void NNAudio_Play(ulong handle);

    [LibraryImport(Lib)]
    internal static partial void NNAudio_Pause(ulong handle);

    [LibraryImport(Lib)]
    internal static partial void NNAudio_Resume(ulong handle);

    [LibraryImport(Lib)]
    internal static partial void NNAudio_Stop(ulong handle);

    [LibraryImport(Lib)]
    internal static partial void NNAudio_Seek(ulong handle, double seconds);

    // ── 状态查询 ──

    [LibraryImport(Lib)]
    internal static partial double NNAudio_GetPlaybackTime(ulong handle);

    [LibraryImport(Lib)]
    internal static partial double NNAudio_GetDuration(ulong handle);

    [LibraryImport(Lib)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static partial bool NNAudio_IsPlaying(ulong handle);

    // ── 参数控制 ──

    [LibraryImport(Lib)]
    internal static partial void NNAudio_SetVolume(ulong handle, float volume);

    [LibraryImport(Lib)]
    internal static partial float NNAudio_GetVolume(ulong handle);

    [LibraryImport(Lib)]
    internal static partial void NNAudio_SetLoop(ulong handle, [MarshalAs(UnmanagedType.I1)] bool loop);

    [LibraryImport(Lib)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static partial bool NNAudio_IsLooping(ulong handle);
}
