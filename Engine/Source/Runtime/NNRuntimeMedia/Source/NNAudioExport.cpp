/**
 * @file NNAudioExport.cpp
 * @brief 音频播放 C ABI 实现——桥接 NN::Core::AudioPlayer（SDL3 + FAudioDecoder）。
 */

#include "../Include/NNAudioExport.h"
#include <NNCore/Interface/HCore.h>
#include <NNMediaCore/Include/Audio.h>
#include <NNRuntimeVFS/Include/VFSService.h>

#include <unordered_map>
#include <mutex>
#include <iostream>

namespace
{
    struct AudioPlayerEntry
    {
        NN::Ref<NN::Core::AudioPlayer> player;
        NN::Ref<NN::Core::AudioClip>   clip;
        std::string filePath;
        double duration = 0.0;
    };

    std::unordered_map<uint64_t, AudioPlayerEntry> g_Players;
    std::mutex g_Mutex;
    uint64_t g_NextHandle = 1;

    AudioPlayerEntry* FindPlayer(uint64_t handle)
    {
        auto it = g_Players.find(handle);
        return it != g_Players.end() ? &it->second : nullptr;
    }
}

NN_RUNTIME_MEDIA_API uint64_t NNAudio_CreatePlayer(void)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    uint64_t handle = g_NextHandle++;
    AudioPlayerEntry entry;
    entry.player = NN::MakeRef<NN::Core::AudioPlayer>();
    g_Players[handle] = std::move(entry);
    std::cout << "[NNAudio] CreatePlayer handle=" << handle << std::endl;
    return handle;
}

NN_RUNTIME_MEDIA_API void NNAudio_DestroyPlayer(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto it = g_Players.find(handle);
    if (it != g_Players.end())
    {
        it->second.player->Stop();
        g_Players.erase(it);
        std::cout << "[NNAudio] DestroyPlayer handle=" << handle << std::endl;
    }
}

NN_RUNTIME_MEDIA_API bool NNAudio_LoadFile(uint64_t handle, const char* filePathUtf8)
{
    std::lock_guard<std::mutex> lock(g_Mutex);

    auto* entry = FindPlayer(handle);
    if (!entry || !entry->player || !filePathUtf8)
    {
		H_LOG_WARN("Invalid player or file path");
		return false;
    }

    std::string filePath = filePathUtf8;
    std::cout << "[NNAudio] LoadFile handle=" << handle << " path=" << filePath << std::endl;

    if (entry->player->IsPlaying())
        entry->player->Stop();

    auto& vfs = NN::Runtime::VFS::VFSService::GetInstance();
    auto clip = NN::MakeRef<NN::Core::AudioClip>();

    if (!clip->Open(vfs, filePath))
    {
        std::cerr << "[NNAudio] AudioClip::Open failed: " << filePath << std::endl;
        return false;
    }

    if (!entry->player->OpenAudioClip(clip))
    {
        std::cerr << "[NNAudio] AudioPlayer::OpenAudioClip failed" << std::endl;
        return false;
    }

    entry->clip = clip;
    entry->filePath = filePath;
    entry->duration = entry->player->GetDuration();

    std::cout << "[NNAudio] LoadFile success duration=" << entry->duration << "s" << std::endl;
    return true;
}

NN_RUNTIME_MEDIA_API void NNAudio_Play(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->Play();
}

NN_RUNTIME_MEDIA_API void NNAudio_Pause(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->Pause();
}

NN_RUNTIME_MEDIA_API void NNAudio_Resume(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->Restore();
}

NN_RUNTIME_MEDIA_API void NNAudio_Stop(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->Stop();
}

NN_RUNTIME_MEDIA_API void NNAudio_Seek(uint64_t handle, double seconds)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->Seek(seconds);
}

NN_RUNTIME_MEDIA_API double NNAudio_GetPlaybackTime(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        return entry->player->GetPlaybackTime();
    return 0.0;
}

NN_RUNTIME_MEDIA_API double NNAudio_GetDuration(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry)
        return entry->duration;
    return 0.0;
}

NN_RUNTIME_MEDIA_API bool NNAudio_IsPlaying(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        return entry->player->IsPlaying();
    return false;
}

NN_RUNTIME_MEDIA_API void NNAudio_SetVolume(uint64_t handle, float volume)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->SetVolume(volume);
}

NN_RUNTIME_MEDIA_API float NNAudio_GetVolume(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        return entry->player->GetVolume();
    return 1.0f;
}

NN_RUNTIME_MEDIA_API void NNAudio_SetLoop(uint64_t handle, bool loop)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        entry->player->SetLoop(loop);
}

NN_RUNTIME_MEDIA_API bool NNAudio_IsLooping(uint64_t handle)
{
    std::lock_guard<std::mutex> lock(g_Mutex);
    auto* entry = FindPlayer(handle);
    if (entry && entry->player)
        return entry->player->IsLooping();
    return false;
}
