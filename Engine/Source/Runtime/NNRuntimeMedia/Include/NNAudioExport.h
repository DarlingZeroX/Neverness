/**
 * @file NNAudioExport.h
 * @brief 音频播放 C ABI 导出——供 C# P/Invoke 调用。
 *
 * 内部桥接 NN::Core::AudioPlayer（SDL3 + FAudioDecoder），
 * SDL3 不暴露到 C# 层。
 */

#pragma once
#include <cstdint>
#include "../NNRuntimeMediaExport.h"

#ifdef __cplusplus
extern "C" {
#endif

/// 创建音频播放器实例，返回 opaque handle。
/// 返回 0 表示失败。
NN_RUNTIME_MEDIA_API uint64_t NNAudio_CreatePlayer(void);

/// 销毁音频播放器实例。
NN_RUNTIME_MEDIA_API void NNAudio_DestroyPlayer(uint64_t handle);

/// 加载音频文件（WAV/MP3/OGG/FLAC）。
/// @param handle  播放器 handle
/// @param filePathUtf8  UTF-8 编码的文件路径（VFS 虚拟路径或绝对路径）
/// @return 成功返回 true
NN_RUNTIME_MEDIA_API bool NNAudio_LoadFile(uint64_t handle, const char* filePathUtf8);

/// 从头播放。
NN_RUNTIME_MEDIA_API void NNAudio_Play(uint64_t handle);

/// 暂停播放。
NN_RUNTIME_MEDIA_API void NNAudio_Pause(uint64_t handle);

/// 恢复播放（从暂停位置继续）。
NN_RUNTIME_MEDIA_API void NNAudio_Resume(uint64_t handle);

/// 停止播放并释放 SDL3 设备。
NN_RUNTIME_MEDIA_API void NNAudio_Stop(uint64_t handle);

/// 跳转到指定时间（秒）。
NN_RUNTIME_MEDIA_API void NNAudio_Seek(uint64_t handle, double seconds);

/// 获取当前播放时间（秒）。
NN_RUNTIME_MEDIA_API double NNAudio_GetPlaybackTime(uint64_t handle);

/// 获取音频总时长（秒）。
NN_RUNTIME_MEDIA_API double NNAudio_GetDuration(uint64_t handle);

/// 是否正在播放。
NN_RUNTIME_MEDIA_API bool NNAudio_IsPlaying(uint64_t handle);

/// 设置音量 [0.0, 1.0]。
NN_RUNTIME_MEDIA_API void NNAudio_SetVolume(uint64_t handle, float volume);

/// 获取当前音量。
NN_RUNTIME_MEDIA_API float NNAudio_GetVolume(uint64_t handle);

/// 设置循环播放。
NN_RUNTIME_MEDIA_API void NNAudio_SetLoop(uint64_t handle, bool loop);

/// 是否循环播放。
NN_RUNTIME_MEDIA_API bool NNAudio_IsLooping(uint64_t handle);

#ifdef __cplusplus
}
#endif
