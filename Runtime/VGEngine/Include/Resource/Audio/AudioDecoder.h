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

#include "../../Core/Core.h"
#include "../../EngineConfig.h"

#include "../Common/FFmpegContext.h"
#include "AudioRingBuffer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

namespace VisionGal {

	// 视频解码器类
	class VG_ENGINE_API AudioDecoder {
	public:
		AudioDecoder();
		~AudioDecoder();

		bool Open(const std::string& filePath);
		void Close();
		double GetDuration() const;

		AudioRingBuffer* GetAudioBuffer() const { return m_AudioRingBuffer.get(); }
		double GetAudioClock() const { return audioClock; }

		void StartDecode();		// 开始解码
		void StopDecode();		// 暂停解码
		void SetLoopDecode(bool enable);	// 设置循环解码
		bool IsLoopDecode() const;			// 是否循环解码
		void SetPauseDecode(bool pause);	// 设置暂停解码
		bool IsPauseDecode() const;			// 是否暂停解码
	private:
		void AudioThread();
	private:
		Ref<FFmpegContext> m_fContext = nullptr;
		bool m_EnableDecodeLoop = false;
		bool m_IsPauseDecode = false;

		//音频
		int m_AudioStreamIndex;
		int m_AudioMaxSamples = 1024;
		Ref<AudioRingBuffer> m_AudioRingBuffer;
		AVCodecContext* actx = nullptr;
		AVFrame* aframe = nullptr;
		SwrContext* swr = nullptr;
		uint8_t* audioBuf = nullptr;

		std::thread audioThread;
		std::atomic<bool> running = false;
		double audioClock = 0.0;
	};

}
