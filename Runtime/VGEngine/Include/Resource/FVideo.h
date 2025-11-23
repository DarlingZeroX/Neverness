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
#include "../Core/Core.h"
#include "../EngineConfig.h"
#include "../Render/Sprite.h"
#include "Video/FVideoDecoder.h"

namespace VisionGal {
	class VG_ENGINE_API FVideoClip : public VGEngineResource
	{
	public:
		FVideoClip() = default;
		FVideoClip(const FVideoClip&) = delete;
		FVideoClip& operator=(const FVideoClip&) = delete;
		FVideoClip(FVideoClip&&) noexcept = default;
		FVideoClip& operator=(FVideoClip&&) noexcept = default;
		~FVideoClip() override = default;

		bool Open(const String& filePath);
		float2 GetSize() const;

		FVideoDecoder* GetDecoder() const;
	private:
		Ref<FVideoDecoder> m_VideoDecoder;
	};

	// 视频播放器类 - 封装解码和渲染
	class VG_ENGINE_API FVideoPlayer 
	{
		static void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
	public:
		FVideoPlayer();
		~FVideoPlayer();

		static Ref<FVideoPlayer> CreatePlayer(const Ref<FVideoClip>& clip);

		bool Open(const Ref<FVideoClip>& clip);
		bool Open(const std::string& filePath);
		void Play();
		void Stop();
		bool IsPlaying() const;
		bool IsLooping() const;									// 是否循环播放
		double GetDuration() const;
		//void PlayBackLoop();
		bool Pause();												// 暂停播放
		bool Restore();											// 恢复播放

		void SetLoop(bool enable);
		bool IsLoop();

		void Update();

		double GetPlaybackTime() const;						// 获取当前播放时间（秒）
		bool Seek(double seconds) ;

		VGFX::ITexture* GetVideoTexture() const { return m_VideoTexture.get(); }
		//Sprite* GetSprite() const { return sprite.get(); }
	private:
		bool PlayAudio();

		void ProcessVideoUpdate(uint8_t* frameData, int& linesize, int width, int height, double pts);

		void FinishPlayAudio(SDL_AudioStream* stream);
		// 处理音频流数据
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);

		// 视频
		Ref<FVideoClip> m_VideoClip;
		Ref<VGFX::ITexture> m_VideoTexture;
		std::mutex m_Mutex;
		uint8_t* m_FrameData = nullptr;
		bool m_VideoClockInitialized = false;
		double m_VideoStartPTS = 0.f;
		std::chrono::time_point<std::chrono::steady_clock>  m_SystemStartTime;

		// 音频
		SDL_AudioStream* m_AudioStream = nullptr;
		SDL_AudioDeviceID m_AudioDev;

		bool m_IsFinished = false;
		bool m_IsPlaying = false;
		float m_Volume = 1.0f;

		size_t m_PlayedBytes = 0;    // 已经被 SDL 播放的 PCM 字节数
		int    m_BytesPerSec = 0;    // 每秒 PCM 字节数（根据 format 计算）
	};


}
