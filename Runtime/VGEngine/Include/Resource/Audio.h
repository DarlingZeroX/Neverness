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
#include "Interface/IAudioClip.h"
#include "Interface/IAudioDecoder.h"
#include "Interface/IAudioPlayer.h"

namespace VisionGal
{
    class VG_ENGINE_API AudioClip : public IAudioClip
    {
    public:
        AudioClip();
        ~AudioClip() override;

        bool Open(const String& filePath);

		IAudioDecoder* GetDecoder() override;
    private:
		Ref<IAudioDecoder> m_AudioDecoder;
    };
	  
    class VG_ENGINE_API AudioPlayer : public IAudioPlayer
	{
        static void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
    public:
        AudioPlayer();
        ~AudioPlayer() override;

		static Ref<AudioPlayer> CreatePlayer(const Ref<AudioClip>& clip);

        bool Init();														// 初始化音频系统
        bool OpenAudioClip(const Ref<IAudioClip>& clip) override;	        // 打开音频片段
        bool Play() override;												// 开始播放
		bool SetLoop(bool enable) override;									// 循环播放
        bool Stop() override;												// 停止播放
		bool IsStop() override;
        bool IsPlaying() const override;									// 是否正在播放音频
        bool IsLooping() const override;									// 是否循环播放
		bool SetVolume(float v) override;									// 设置音量
        float GetVolume() const override;									// 获取音量
		bool Pause() override;												// 暂停播放
		bool Restore() override;											// 恢复播放
		double GetDuration() const override;
		double GetAudioPlaybackTime() const override;						// 获取音频设备的当前播放时间（秒）

		bool Seek(double seconds) override;
    private:
		void FinishPlay(SDL_AudioStream* stream);
        // 处理音频流数据
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);

        Ref<IAudioClip> m_AudioClip = nullptr;
        SDL_AudioStream* m_AudioStream = nullptr;
		SDL_AudioDeviceID m_AudioDev;

		bool m_IsFinished = false;
        bool m_IsPlaying = false;
        float m_Volume = 1.0f;

		size_t m_PlayedBytes = 0;    // 已经被 SDL 播放的 PCM 字节数
		int    m_BytesPerSec = 0;    // 每秒 PCM 字节数（根据 format 计算）
    };
}
