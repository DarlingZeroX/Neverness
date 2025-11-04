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
        void Clear();

        //vfspp::IFilePtr file; // VFSpp 文件句柄
        //SDL_IOStream* ioStream;
        //SDL_AudioSpec audioSpec;
        //uint8_t* audioBuffer;       // 原始WAV数据
        //uint32_t bufferLength;      // 数据总长度
		//VFS::DataRef audioData; // 用于存储音频数据

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
        // 处理音频流数据
        void HandleAudioStream(SDL_AudioStream* stream, int need_bytes);

        Ref<AudioClip> audioClip;
        SDL_AudioStream* audioStream = nullptr;

        uint8_t* bufferCopy = nullptr;// 复制的播放缓冲区
        uint8_t* bufferPosition;    // 当前播放位置
        uint32_t remainingLength;   // 剩余未播放长度

        std::atomic<bool> isPlaying;
        bool m_EnableLoop = false;
        float m_Volume = 1.0f;
    };
}
