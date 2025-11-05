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

		AudioRingBuffer* GetAudioBuffer() const { return audioRingBuffer.get(); }
		double GetAudioClock() const { return audioClock; }

		void StartDecode();
		void StopDecode();
		void SetDecodeLoop(bool enable);
		bool IsDecodeLoop();
	private:
		void AudioThread();
	private:
		Ref<FFmpegContext> m_fContext = nullptr;
		bool m_EnableDecodeLoop = false;

		//音频
		int audioStreamIndex;
		int audioMaxSamples = 1024;
		Ref<AudioRingBuffer> audioRingBuffer;
		AVCodecContext* actx = nullptr;
		AVFrame* aframe = nullptr;
		SwrContext* swr = nullptr;
		uint8_t* audioBuf = nullptr;

		std::thread audioThread;
		std::atomic<bool> running = false;
		double audioClock = 0.0;
	};

}
