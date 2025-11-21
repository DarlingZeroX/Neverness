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
#include <SDL3/SDL_audio.h>

#include "../Interface/IAudioDecoder.h"
#include "../../Core/Core.h"
#include "../../EngineConfig.h"

#include "../FFmpeg/FContext.h"
#include "../FFmpeg/FFmpegContext.h"
#include "../FFmpeg/FSwrContext.h"
#include "../FFmpeg/FFrame.h"
#include "../FFmpeg/FCodecContext.h"
#include "AudioRingBuffer.h"

namespace VisionGal {

	// 视频解码器类
	class VG_ENGINE_API FAudioDecoder: public IAudioDecoder{
	public:
		FAudioDecoder();
		~FAudioDecoder() override;

		bool Open(const VGPath& filePath);

		IAudioDataBuffer* GetAudioBuffer() const override { return m_AudioRingBuffer.get(); }
		double GetAudioClock() const override { return audioClock; }

		void StartDecode() override;					// 开始解码
		void StopDecode() override;						// 暂停解码
		void SetLoopDecode(bool enable) override;		// 设置循环解码
		bool IsLoopDecode() const override;				// 是否循环解码
		void SetPauseDecode(bool pause) override;		// 设置暂停解码
		bool IsPauseDecode() const override;			// 是否暂停解码

		void Close() override;
		double GetDuration() const override;
	private:
		void AudioThread();
	private:
		Ref<FfmpegContext> m_FContext = nullptr;
		Ref<FfmpegAVCodecContext> m_CodecContext = nullptr;
		Ref<FfmpegSwrContext> m_SwrContext = nullptr;

		Ref<AudioRingBuffer> m_AudioRingBuffer;
		Ref<FfmpegAVFrame> m_FfmpegAVFrame;

		std::atomic<bool> m_IsRunning = false;

		bool m_EnableDecodeLoop = false;
		bool m_IsPauseDecode = false;

		//音频
		int m_AudioStreamIndex;
		int m_AudioMaxSamples = 1024;
		uint8_t* m_AudioBuf = nullptr;
		std::thread m_AudioThread;
		double audioClock = 0.0;
	};

}
