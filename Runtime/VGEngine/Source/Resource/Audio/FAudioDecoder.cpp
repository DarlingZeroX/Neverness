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

#include "Resource/Audio/FAudioDecoder.h"
#include "Resource/FFmpeg/FChannelLayout.h"

namespace VisionGal {
	FAudioDecoder::FAudioDecoder()
	{
	}

	FAudioDecoder::~FAudioDecoder()
	{
		if (m_AudioBuf != nullptr)
		{
			av_free(m_AudioBuf);
			m_AudioBuf = nullptr;
		}
	}

	bool FAudioDecoder::Open(const VGPath& filePath)
	{
		m_FContext = FfmpegContext::Create(filePath);

		if (m_FContext == nullptr)
			return false;

		// 查找视频流和音频流
		m_AudioStreamIndex = m_FContext->FindAudioStreamIndex();
		if (m_AudioStreamIndex != -1)				// 音频解码器
		{
			// 音频
			m_CodecContext = CreateRef<FfmpegAVCodecContext>(*m_FContext->GetFormatContext(), m_AudioStreamIndex);

			// 假设 actx 是已经 open2 的 AVCodecContext*
			FfmpegAVChannelLayout in_ch_layout(m_CodecContext->GetCHLayout());
			FfmpegAVChannelLayout out_ch_layout(2);
			m_SwrContext = FfmpegSwrContext::Create(in_ch_layout, out_ch_layout, *m_CodecContext);

			// 创建音频数据缓冲区
			m_AudioRingBuffer = CreateRef<AudioRingBuffer>(2 * 1024 * 1024);

			// 创建Ffmpeg音频缓冲区
			m_AudioMaxSamples = av_rescale_rnd(4096, 44100, m_CodecContext->GetSampleRate(), AV_ROUND_UP);
			av_samples_alloc(&m_AudioBuf, nullptr, 2, m_AudioMaxSamples, AV_SAMPLE_FMT_S16, 0);
		}

		m_FfmpegAVFrame = CreateRef<FfmpegAVFrame>();

		return true;
	}

	bool FAudioDecoder::StartDecode()
	{
		// 先确保旧线程 join 完成
		if (m_AudioThread.joinable())
			m_AudioThread.join();

		m_IsRunning = true;
		m_AudioThread = std::thread(&FAudioDecoder::AudioThread, this);
		return true;
	}

	bool FAudioDecoder::StopDecode()
	{
		m_IsRunning = false;
		if (m_AudioThread.joinable()) 
			m_AudioThread.join();
		return true;
	}

	//bool FAudioDecoder::SetLoopDecode(bool enable)
	//{
	//	m_EnableDecodeLoop = enable;
	//
	//	return RestoreDecode();
	//}
	//
	//bool FAudioDecoder::IsLoopDecode() const
	//{
	//	return m_EnableDecodeLoop;
	//}

	bool FAudioDecoder::PauseDecode(bool pause)
	{
		m_IsPauseDecode = pause;
		return true;
	}

	bool FAudioDecoder::RestoreDecode()
	{
		m_IsPauseDecode = false;

		if (IsRunningDecode() == false)
		{
			return StartDecode();
		}

		return true;
	}

	bool FAudioDecoder::IsPauseDecode() const
	{
		return m_IsPauseDecode;
	}

	bool FAudioDecoder::RestartDecode()
	{
		if (Seek(0) == false)
			return false;

		if (RestoreDecode() == false)
			return false;

		return true;
	}

	void FAudioDecoder::Close()
	{
	}

	double FAudioDecoder::GetDuration() const
	{
		auto* formatCtx = m_FContext->GetFormatContext();
		if (formatCtx->GetDuration() != AV_NOPTS_VALUE)
			return (double)formatCtx->GetDuration() / AV_TIME_BASE;

		// fallback：使用 stream
		AVStream* st = formatCtx->GetStream(m_AudioStreamIndex);
		if (st->duration != AV_NOPTS_VALUE)
			return static_cast<double>(st->duration) * st->time_base.num / st->time_base.den;

		return 0.f;
	}

	bool FAudioDecoder::Seek(double seconds)
	{
		if (!m_FContext || m_AudioStreamIndex < 0)
			return false;

		// 先对线程进行加锁，防止与解码线程访问冲突
		{
			std::unique_lock lock(m_AudioControlMutex);

			auto* formatCtx = m_FContext->GetFormatContext();
			int64_t timestamp = static_cast<int64_t>(seconds / av_q2d(formatCtx->GetStream(m_AudioStreamIndex)->time_base));
			//int ret = formatCtx->SeekFrame(m_AudioStreamIndex, timestamp, AVSEEK_FLAG_ANY);
			//if (ret < 0) {
			//	std::cerr << "[FAudioDecoder] av_seek_frame failed\n";
			//	return false;
			//}

			// 尝试回到文件头并继续解码
			// 1. 对音频流使用时间戳 0，向后搜索
			int ret = formatCtx->SeekFrame(m_AudioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
			if (ret < 0)
			{
				// 如果按流索引失败，尝试全局 seek
				ret = formatCtx->SeekFrame(-1, timestamp, AVSEEK_FLAG_BACKWARD);
				return ret >= 0;
			}

			// 2. 清空 RingBuffer
			auto ring = GetAudioBuffer();
			ring->Reset();
		}

		// 3. 恢复解码，在解锁之后
		RestoreDecode();

		// 清空解码队列
		//m_DecodedFrames.clear();

		// decoder 状态重置
		//m_CurrentPts = seconds;

		return true;
	}

	bool FAudioDecoder::IsRunningDecode() const
	{
		return m_IsRunning;
	}

	void FAudioDecoder::AudioThread()
	{
		while (IsRunningDecode()) {

			// 暂停
			if (m_IsPauseDecode) {

				if (IsRunningDecode() == false)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			// 读取帧
			FfmpegAVPacket pkt;
			FfmpegAVFormatContext* formatContext = m_FContext->GetFormatContext();
			{
				// 音频控制锁, 因为Seek会对formatContext进行操作,需要防止多线程同时访问
				int readResult = 0;
				{
					std::unique_lock<std::mutex> lock(m_AudioControlMutex);
					readResult = formatContext->ReadFrame(pkt);
				}
				if (readResult < 0)
				{
					// 到达文件结尾
					//if (m_EnableDecodeLoop && m_IsRunning)
					//{
					//	Seek(0);
					//	// 尝试回到文件头并继续解码
					//	// 对音频流使用时间戳 0，向后搜索
					//	//int ret = formatContext->SeekFrame(m_AudioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					//	//if (ret < 0)
					//	//{
					//	//	// 如果按流索引失败，尝试全局 seek
					//	//	formatContext->SeekFrame(-1, 0, AVSEEK_FLAG_BACKWARD);
					//	//}
					//
					//	// 清空解码器内部缓冲区，避免残留帧
					//	if (m_CodecContext)
					//		m_CodecContext->FlushBuffers();
					//
					//	// 重置时钟（从头播放）
					//	audioClock = 0.0;
					//
					//	// 继续循环读取
					//	std::this_thread::sleep_for(std::chrono::milliseconds(1));
					//	continue;
					//}
					//else
					//{
						// 非循环：标记写入结束并退出线程
						if (m_AudioRingBuffer)
							m_AudioRingBuffer->WriteFinish();
						m_IsRunning = false;
						break;
					//}
				}
			}

			// 如果是音频流就解码
			if (pkt.GetStreamIndex() == m_AudioStreamIndex)
			{
				m_CodecContext->SendPacket(pkt);
				while (m_CodecContext->ReceiveFrame(*m_FfmpegAVFrame) >= 0) {
					//int samples = swr_convert(swr, &m_AudioBuf, m_AudioMaxSamples, (const uint8_t**)m_FfmpegAVFrame->data, m_FfmpegAVFrame->GetNumberOfSamples());
					int samples = swr_convert(
						m_SwrContext->GetPtr(), 
						&m_AudioBuf, 
						m_AudioMaxSamples, 
						m_FfmpegAVFrame->GetDataAddress(), 
						m_FfmpegAVFrame->GetNumberOfSamples()
					);

					if (samples > 0) {
						int channels = 2;
						int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
						int bytes = samples * channels * bps;
						int bufSize = av_samples_get_buffer_size(
							nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);

						m_AudioRingBuffer->Write(m_AudioBuf, bufSize);

						AVRational tb = formatContext->GetStream(m_AudioStreamIndex)->time_base;
						audioClock = m_FfmpegAVFrame->GetBestEffortTimestamp() * av_q2d(tb);
					}
					//std::cout << m_IsRunning << std::endl;
					// 等待音频缓冲区有空间
					while (m_AudioRingBuffer && m_AudioRingBuffer->IsAlmostFull())
					{
						//std::cout << m_IsRunning << std::endl;
						if (IsRunningDecode() == false)
							return;
						//std::cout << m_IsPauseDecode << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			}
		}
	}
}
