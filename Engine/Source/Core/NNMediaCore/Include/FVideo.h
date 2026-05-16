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
#include "HMediaConfig.h"
#include "../Interface/VideoInterface.h"
#include <NNCore/Interface/HCore.h>
#include <NNFileSystem/Include/VFS/VirtualFileSystem.hpp>
#include "SDL/SDLAudio.h"

namespace NN::Core {
	class H_MEDIA_API FVideoClip : public IVideoClip
	{
	public:
		FVideoClip() = default;
		~FVideoClip() override = default;
		FVideoClip(const FVideoClip&) = delete;
		FVideoClip& operator=(const FVideoClip&) = delete;
		FVideoClip(FVideoClip&&) noexcept = default;
		FVideoClip& operator=(FVideoClip&&) noexcept = default;

		bool Open(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath);
		uint2 GetSize() const override;

		IVideoDecoder* GetDecoder() override;
	private:
		Ref<IVideoDecoder> m_VideoDecoder;
	};

	// 视频播放器类 - 封装解码和渲染
	class H_MEDIA_API FVideoPlayer : public IVideoPlayer
	{
		static void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
	public:
		FVideoPlayer();
		~FVideoPlayer() override;

		static Ref<FVideoPlayer> CreatePlayer(const Ref<IVideoClip>& clip);

		bool Open(const Ref<IVideoClip>& clip) override;				// 打开音频片段
		void Play() override;											// 播放视频
		void Stop() override;											// 暂停播放
		bool IsPlaying() const override;								// 是否正在播放视频
		double GetDuration() const override;							// 获取视频时长
		bool Pause() override;											// 暂停播放
		bool Restore() override;										// 恢复播放
		bool IsLooping() const override;								// 是否循环播放
		void SetLoop(bool loop) override;								// 设置循环播放
		double GetPlaybackTime() const override;						// 获取当前播放时间（秒）
		bool Seek(double seconds) override;								// 视频跳转
		bool RestartPlay() override;
		bool SetVolume(float v) override;									// 设置音量
		float GetVolume() const override;									// 获取音量
		float GetVideoWidth() const override;
		float GetVideoHeight() const override;
		uint8_t* GetFrameData() override;

		void Update() override;											// 更新
		//VGFX::ITexture* GetVideoTexture() const override;				// 获取视频纹理
		//Ref<VGFX::ITexture> GetVideoTextureRef() const override;				// 获取视频纹理
	protected:
		bool PlayAudio();
		void ProcessVideoUpdate(uint8_t* frameData, int& linesize, int width, int height, double pts);
		void FinishPlayAudio(SDL_AudioStream* stream);
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);				// 处理音频流数据
	private:
		// 状态
		//bool m_IsFinished = false;
		bool m_IsPlaying = false;
		bool m_IsLoopPlay = false;
		std::mutex m_Mutex;

		// 视频
		Ref<IVideoClip> m_VideoClip;
		//Ref<VGFX::ITexture> m_VideoTexture;
		uint8_t* m_FrameData = nullptr;
		bool m_VideoClockInitialized = false;
		double m_VideoStartPTS = 0.f;
		std::chrono::time_point<std::chrono::steady_clock>  m_SystemStartTime;

		// 音频
		SDL3AudioStream m_AudioStream;
		SDL3AudioDevice m_AudioDevice;
		float m_Volume = 1.0f;
		size_t m_PlayedBytes = 0;    // 已经被 SDL 播放的 PCM 字节数
		int    m_BytesPerSec = 0;    // 每秒 PCM 字节数（根据 format 计算）
	};


}
