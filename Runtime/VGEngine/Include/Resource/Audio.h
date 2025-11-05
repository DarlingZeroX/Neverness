#pragma once
#include "../Core/VFS.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <iostream>
#include <string>
#include <atomic>
#include "Audio/AudioDecoder.h"

namespace VisionGal
{
    class VG_ENGINE_API AudioClip : public VGEngineResource
    {
    public:
        AudioClip();
        ~AudioClip() override;

        bool Open(const String& filePath);
		AudioDecoder audioDecoder;
    private:
        bool LoadAudio();
    };
	  
    class AudioPlayer {
        friend void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
    public:
        AudioPlayer();
        ~AudioPlayer();

        // 初始化音频系统
        bool Init();
        // 打开音频片段
        bool OpenAudioClip(const Ref<AudioClip>& clip);
        // 开始播放
        bool Play();
        // 循环播放
        void SetLoop(bool enable);
        // 停止播放
        void Stop();
        // 是否正在播放音频
        bool IsPlayingAudio() const;
        // 是否循环播放
        bool IsLooping() const;
        // 设置音量
        void SetVolume(float v);
        // 获取音量
        float GetVolume() const;
    private:
		void FinishPlay(SDL_AudioStream* stream);
        // 处理音频流数据
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);

        Ref<AudioClip> audioClip;
        SDL_AudioStream* audioStream = nullptr;
		SDL_AudioDeviceID audioDev;

        std::atomic<bool> isPlaying;
        float m_Volume = 1.0f;
    };
}
