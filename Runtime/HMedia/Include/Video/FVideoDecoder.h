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
#include "../HMediaConfig.h"
#include "../../Interface/VideoDecoderInterface.h"
#include "../Audio/AudioRingBuffer.h"

#include "../FFmpeg/FContext.h"
#include "../FFmpeg/FPacket.h"
#include "../FFmpeg/FSwrContext.h"
#include "../FFmpeg/FSwsContext.h"
#include "../FFmpeg/FFrame.h"
#include "../FFmpeg/FCodecContext.h"

namespace Horizon {

	// 视频解码器类
	class H_MEDIA_API FVideoDecoder: public IVideoDecoder
	{
	public:
		FVideoDecoder();
		~FVideoDecoder() override;

		bool Open(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath);
		bool StartDecode() override;
		bool StopDecode() override;
		bool Seek(double seconds) override;

		bool IsRunningDecode() const override;							// 是否正在解码
		int GetVideoWidth() const override { return m_VideoWidth; }		// 获取视频宽度
		int GetVideoHeight() const override { return m_VideoHeight; }	// 获取视频高度
		bool HasAudioStream() const override;							// 是否有音频流

		bool PauseDecode(bool pause) override;			// 设置暂停解码
		bool RestoreDecode() override;					// 设置恢复解码
		bool RestartDecode() override;					// 
		bool IsPauseDecode() const override;			// 是否暂停解码
		bool IsFinishedDecode() const override;

		double GetPlaybackTime() const override;

		IAudioDataBuffer* GetAudioBuffer() const override { return m_AudioRingBuffer.get(); }
		double GetDuration() const override;

		//std::function<void(uint8_t* frameData, int& lineSize, int width, int height, double pts)> OnVideoDataUpdate = nullptr;
	protected:
		void DemuxLoop();									// Demux循环
		void AudioDecodeLoop();								// 音频解码循环
		bool VideoDecodeLoop();								// 视频解码循环

		bool InitializeAudio(FfmpegContext& context);
		bool InitializeVideo(FfmpegContext& context);

		void OnFinishedDecode();
	private:
		Ref<FfmpegContext> m_FContext = nullptr;
		AVRational m_TimeBase;

		// 状态
		std::atomic<bool> m_IsRunning = false;
		bool m_IsDemuxFinished = false;
		bool m_IsFinishedDecode = false;
		//bool m_EnableDecodeLoop = false;
		bool m_IsPauseDecode = false;

		// 音频
		Ref<FfmpegAVCodecContext> m_AudioCodecContext = nullptr;
		Ref<FfmpegSwrContext> m_AudioSwrContext = nullptr;
		Ref<AudioRingBuffer> m_AudioRingBuffer;
		Ref<FfmpegAVFrame> m_AudioFrame;
		int m_AudioStreamIndex = 0;
		int m_AudioMaxSamples = 1024;
		uint8_t* m_AudioBuf = nullptr;
		std::atomic<double> m_AudioClock{ 0.0 };

		// 视频
		int m_VideoWidth = 0;
		int m_VideoHeight = 0;
		int m_VideoStreamIndex = 0;
		std::atomic<double> m_VideoClock{ 0.0 };
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
