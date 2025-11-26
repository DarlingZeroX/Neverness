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

#include "Resource/Audio/AudioDecoder.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include "Core/VFS.h"

namespace VisionGal
{
	AudioDecoder::AudioDecoder()
	{

		//audioBuf[0] = nullptr;
		//audioBuf[1] = nullptr;
	}

	AudioDecoder::~AudioDecoder()
	{
		Close();
	}

	bool AudioDecoder::Open(const std::string& filePath)
	{
		m_fContext = FFmpegContext::Create(filePath);

		if (m_fContext == nullptr)
			return false;

		// 查找视频流和音频流
		//AVCodecParameters* codecParameters = FindStream();
		m_AudioStreamIndex = m_fContext->FindAudioStreamIndex();

		// 音频解码器
		if (m_AudioStreamIndex != -1)
		{
			// 音频
			actx = avcodec_alloc_context3(avcodec_find_decoder(m_fContext->formatContext->streams[m_AudioStreamIndex]->codecpar->codec_id));
			avcodec_parameters_to_context(actx, m_fContext->formatContext->streams[m_AudioStreamIndex]->codecpar);
			avcodec_open2(actx, nullptr, nullptr);

			// 假设 actx 是已经 open2 的 AVCodecContext*
			AVChannelLayout in_ch_layout = actx->ch_layout;
			AVChannelLayout out_ch_layout;
			av_channel_layout_default(&out_ch_layout, 2);

			swr = nullptr;
			if (swr_alloc_set_opts2(
				&swr,
				&out_ch_layout, AV_SAMPLE_FMT_S16, 44100,
				&in_ch_layout, actx->sample_fmt, actx->sample_rate,
				0, nullptr) < 0)
			{
				std::cerr << "Failed to alloc swr" << std::endl;
				return false;
			}

			if (swr_init(swr) < 0) {
				std::cerr << "Failed to init swr" << std::endl;
				return false;
			}

			m_AudioRingBuffer = CreateRef<AudioRingBuffer>(2 * 1024 * 1024);



			//SDL_PauseAudioDevice(audioDev);

			//audioMaxSamples = 1024;
			m_AudioMaxSamples = av_rescale_rnd(4096, 44100, actx->sample_rate, AV_ROUND_UP);

			av_samples_alloc(&audioBuf, nullptr, 2, m_AudioMaxSamples, AV_SAMPLE_FMT_S16, 0);
		}

		// 分配帧和包
		aframe = av_frame_alloc();
		if (!aframe) {
			std::cerr << "Failed to allocate frame" << std::endl;
			return false;
		}

		return true;
	}

	void AudioDecoder::SetLoopDecode(bool enable)
	{
		m_EnableDecodeLoop = enable;
	}

	bool AudioDecoder::IsLoopDecode() const
	{
		return m_EnableDecodeLoop;
	}

	void AudioDecoder::SetPauseDecode(bool pause)
	{
		m_IsPauseDecode = pause;
	}

	bool AudioDecoder::IsPauseDecode() const
	{
		return m_IsPauseDecode;
	}

	void AudioDecoder::AudioThread()
	{
		while (running) {

			// 暂停
			if (m_IsPauseDecode) {

				if (running == false)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			// 读取帧
			AVPacket* pkt = av_packet_alloc();
			if (av_read_frame(m_fContext->formatContext, pkt) < 0)
			{
				av_packet_free(&pkt);

				// 到达文件结尾
				if (m_EnableDecodeLoop && running)
				{
					// 尝试回到文件头并继续解码
					// 对音频流使用时间戳 0，向后搜索
					int ret = av_seek_frame(m_fContext->formatContext, m_AudioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					if (ret < 0)
					{
						// 如果按流索引失败，尝试全局 seek
						av_seek_frame(m_fContext->formatContext, -1, 0, AVSEEK_FLAG_BACKWARD);
					}

					// 清空解码器内部缓冲区，避免残留帧
					if (actx)
						avcodec_flush_buffers(actx);

					// 重置时钟（从头播放）
					audioClock = 0.0;

					// 继续循环读取
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				else
				{
					// 非循环：标记写入结束并退出线程
					if (m_AudioRingBuffer)
						m_AudioRingBuffer->WriteFinish();
					break;
				}
			}

			// 如果是音频流就解码
			if (pkt->stream_index == m_AudioStreamIndex)
			{
				avcodec_send_packet(actx, pkt);
				av_packet_free(&pkt);
				while (avcodec_receive_frame(actx, aframe) >= 0) {
					int samples = swr_convert(swr, &audioBuf, m_AudioMaxSamples, (const uint8_t**)aframe->data, aframe->nb_samples);

					if (samples > 0) {
						int channels = 2;
						int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
						int bytes = samples * channels * bps;
						int bufSize = av_samples_get_buffer_size(
							nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);
						m_AudioRingBuffer->Write(audioBuf, bufSize);

						AVRational tb = m_fContext->formatContext->streams[m_AudioStreamIndex]->time_base;
						audioClock = aframe->best_effort_timestamp * av_q2d(tb);
					}

					// 等待音频缓冲区有空间
					while (m_AudioRingBuffer && m_AudioRingBuffer->IsAlmostFull())
					{
						if (!running)
							return;

						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			}
			else
				av_packet_free(&pkt);
		}
	}

	void AudioDecoder::StartDecode()
	{
		running = true;
		audioThread = std::thread(&AudioDecoder::AudioThread, this);
	}

	void AudioDecoder::StopDecode()
	{
		running = false;
		if (audioThread.joinable()) audioThread.join();
	}

	void AudioDecoder::Close()
	{
		StopDecode();

		// 音频
		if (actx) {
			avcodec_free_context(&actx);
			actx = nullptr;
		}

		if (aframe) {
			av_frame_free(&aframe);
			aframe = nullptr;
		}

		if (swr) {
			swr_free(&swr);
			swr = nullptr;
		}

		if (audioBuf)
		{
			av_freep(&audioBuf);
			audioBuf = nullptr;
		}

	}

	double AudioDecoder::GetDuration() const
	{
		if (!m_fContext->formatContext) return 0.0;
		return m_fContext->formatContext->duration * av_q2d(AV_TIME_BASE_Q);
	}
}
