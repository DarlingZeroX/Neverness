/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "../EngineConfig.h"
#include "../Core/Core.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include "Interface/IAudioDecoder.h"

namespace VisionGal
{
    class VG_ENGINE_API AudioClip : public VGEngineResource
    {
    public:
        AudioClip();
        ~AudioClip() override;

        bool Open(const String& filePath);

		IAudioDecoder* GetDecoder() const;
    private:
		Ref<IAudioDecoder> m_AudioDecoder;
    };
	  
    class VG_ENGINE_API AudioPlayer {
        static void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
    public:
        AudioPlayer();
        ~AudioPlayer();

		static Ref<AudioPlayer> CreatePlayer(const Ref<AudioClip>& clip);

        bool Init();											 // 初始化音频系统
        bool OpenAudioClip(const Ref<AudioClip>& clip);	         // 打开音频片段
        bool Play();											 // 开始播放
		bool SetLoop(bool enable);								 // 循环播放
        bool Stop();											 // 停止播放
		bool IsStop();
        bool IsPlaying() const;			 // 是否正在播放音频
        bool IsLooping() const;			 // 是否循环播放
		bool SetVolume(float v);		 // 设置音量
        float GetVolume() const;		 // 获取音量
		bool Pause();					 // 暂停播放
		bool Restore();					 // 恢复播放
		double GetDuration();
    private:
		void FinishPlay(SDL_AudioStream* stream);
        // 处理音频流数据
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);

        Ref<AudioClip> m_AudioClip = nullptr;
        SDL_AudioStream* m_AudioStream = nullptr;
		SDL_AudioDeviceID m_AudioDev;

		bool m_IsStop = false;
        bool m_IsPlaying = false;
        float m_Volume = 1.0f;
    };
}
