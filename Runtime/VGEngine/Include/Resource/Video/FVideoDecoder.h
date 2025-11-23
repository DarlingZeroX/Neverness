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
#include "../../Core/Core.h"
#include "../../EngineConfig.h"

#include "../Common/FFmpegIOContext.h"
#include "../Audio/AudioRingBuffer.h"

#include "../FFmpeg/FContext.h"
#include "../FFmpeg/FPacket.h"
#include "../FFmpeg/FFmpegContext.h"
#include "../FFmpeg/FSwrContext.h"
#include "../FFmpeg/FSwsContext.h"
#include "../FFmpeg/FFrame.h"
#include "../FFmpeg/FCodecContext.h"

namespace VisionGal {

	// 视频解码器类
	class VG_ENGINE_API FVideoDecoder {
	public:
		FVideoDecoder();
		~FVideoDecoder();

		bool Open(const std::string& filePath);
		bool StartDecode();
		bool StopDecode();
		bool Seek(double seconds);

		bool IsRunningDecode() const;
		int GetWidth() const { return m_VideoWidth; }		// 获取视频宽度
		int GetHeight() const { return m_VideoHeight; }		// 获取视频高度

		void DemuxLoop();									// Demux循环
		void AudioDecodeLoop();								// 音频解码循环
		bool VideoDecodeLoop();								// 视频解码循环

		void SetLoopDecode(bool enable);					// 设置循环解码
		bool IsLoopDecode() const;							// 是否循环解码
		bool PauseDecode(bool pause);			// 设置暂停解码
		bool RestoreDecode();					// 设置恢复解码
		bool RestartDecode();					// 
		bool IsPauseDecode() const;			// 是否暂停解码

		AudioRingBuffer* GetAudioBuffer() const { return m_AudioRingBuffer.get(); }
		double GetDuration() const ;

		bool HasAudioStream() const;

		std::function<void(uint8_t* frameData, int& lineSize, int width, int height, double pts)> OnVideoDataUpdate = nullptr;
	protected:
		//AVCodecParameters* FindStream();
		//bool OpenVideoDecoder(const AVCodecParameters* codecParameters);

		bool InitializeAudio(FfmpegContext& context);
		bool InitializeVideo(FfmpegContext& context);
	private:
		Ref<FfmpegContext> m_FContext = nullptr;
		AVRational m_TimeBase;

		// 状态
		std::atomic<bool> m_IsRunning = false;
		bool m_IsDemuxFinished = false;
		bool m_EnableDecodeLoop = false;
		bool m_IsPauseDecode = false;

		// 音频
		Ref<FfmpegAVCodecContext> m_AudioCodecContext = nullptr;
		Ref<FfmpegSwrContext> m_AudioSwrContext = nullptr;
		Ref<AudioRingBuffer> m_AudioRingBuffer;
		Ref<FfmpegAVFrame> m_AudioFrame;
		int m_AudioStreamIndex = 0;
		int m_AudioMaxSamples = 1024;
		uint8_t* m_AudioBuf = nullptr;

		// 视频
		int m_VideoWidth = 0;
		int m_VideoHeight = 0;
		int m_VideoStreamIndex = 0;
		Ref<FfmpegAVCodecContext> m_VideoCodecContext = nullptr;
		Ref<FfmpegSwsContext> m_VideoSwsContext = nullptr;
		Ref<FfmpegAVFrame> m_VideoFrame;
		Ref<FfmpegAVFrame> m_VideoRGBFrame;
		Ref<FfmpegBuffer> m_RGBBuffer;

		// 包队列
		std::queue<Ref<FfmpegAVPacket>> m_VideoPacketQueue;
		std::queue<Ref<FfmpegAVPacket>> m_AudioPacketQueue;

		// 锁
		mutable std::mutex m_Mutex;
		std::condition_variable m_Condition;

		// 线程
		std::thread m_DemuxThread;
		std::thread m_AudioThread;
		std::thread m_VideoThread;
	};

}
