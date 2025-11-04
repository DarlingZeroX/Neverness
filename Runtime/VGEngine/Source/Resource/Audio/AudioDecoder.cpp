#include "Resource/Audio/AudioDecoder.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include "Core/VFS.h"

namespace VisionGal
{
	AudioDecoder::AudioDecoder()
	{

		audioBuf[0] = nullptr;
		audioBuf[1] = nullptr;
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
		audioStreamIndex = m_fContext->FindAudioStreamIndex();

		// 音频解码器
		if (audioStreamIndex != -1)
		{
			// 音频
			actx = avcodec_alloc_context3(avcodec_find_decoder(m_fContext->formatContext->streams[audioStreamIndex]->codecpar->codec_id));
			avcodec_parameters_to_context(actx, m_fContext->formatContext->streams[audioStreamIndex]->codecpar);
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

			audioRingBuffer = CreateRef<AudioRingBuffer>(2 * 1024 * 1024);

			auto MyAudioCallback = [](void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
				{
					auto* ring = static_cast<AudioRingBuffer*>(userdata);
					size_t frame_size = 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); // 2ch s16

					while (additional_amount >= (int)frame_size && ring->Available() >= frame_size) {
						uint8_t temp[4096];
						size_t to_read = std::min(sizeof(temp), (size_t)additional_amount);

						// 保证读取的是完整帧数
						to_read = (to_read / frame_size) * frame_size;

						if (to_read == 0) break;

						size_t read = ring->Read(temp, to_read);

						// 再次对齐防止 AudioRingBuffer 只返回部分
						read = (read / frame_size) * frame_size;

						if (read > 0) {
							SDL_PutAudioStreamData(stream, temp, (int)read);
							additional_amount -= (int)read;
						}
						else {
							break;
						}
					}

					if (ring->IsFinish())
					{
						SDL_PauseAudioStreamDevice(stream);
						return;
					}

					if (ring->Available() < frame_size * 10) {
						if (ring->IsWriteFinish())
						{
							SDL_PauseAudioStreamDevice(stream);
						}
						else
						{
							std::cerr << "[Audio] Warning: underrun imminent, ring buffer low" << std::endl;
						}
					}
				};

			if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
				std::cerr << "音频初始化失败: " << SDL_GetError() << std::endl;
				return false;
			}

			SDL_AudioSpec spec{};
			spec.freq = 44100;
			spec.format = SDL_AUDIO_S16;
			spec.channels = 2;

			// 1. 打开默认输出设备
			audioDev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
			if (!audioDev) {
				std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
				return false;
			}

			// 2. 用 audioDev 打开 stream
			audioStream = SDL_OpenAudioDeviceStream(audioDev, &spec, MyAudioCallback, audioRingBuffer.get());
			if (!audioStream) {
				std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
				return false;
			}

			//SDL_PauseAudioDevice(audioDev);

			//audioMaxSamples = 1024;
			audioMaxSamples = av_rescale_rnd(4096, 44100, actx->sample_rate, AV_ROUND_UP);

			av_samples_alloc(audioBuf, nullptr, 2, audioMaxSamples, AV_SAMPLE_FMT_S16, 0);
		}

		// 分配帧和包
		aframe = av_frame_alloc();
		if (!aframe) {
			std::cerr << "Failed to allocate frame" << std::endl;
			return false;
		}

		return true;
	}

	void AudioDecoder::AudioThread()
	{
		while (running) {
			AVPacket* pkt = av_packet_alloc();
			if (av_read_frame(m_fContext->formatContext, pkt) < 0)
			{
				audioRingBuffer->WriteFinish();
				av_packet_free(&pkt);
				break;
			}

			if (pkt->stream_index == audioStreamIndex)
			{
				avcodec_send_packet(actx, pkt);
				av_packet_free(&pkt);
				while (avcodec_receive_frame(actx, aframe) >= 0) {
					int samples = swr_convert(swr, audioBuf, audioMaxSamples, (const uint8_t**)aframe->data, aframe->nb_samples);

					if (samples > 0) {
						int channels = 2;
						int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
						int bytes = samples * channels * bps;
						int bufSize = av_samples_get_buffer_size(
							nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);
						audioRingBuffer->Write(audioBuf[0], bufSize);

						AVRational tb = m_fContext->formatContext->streams[audioStreamIndex]->time_base;
						audioClock = aframe->best_effort_timestamp * av_q2d(tb);
					}

					// 等待音频缓冲区有空间
					while (audioRingBuffer && audioRingBuffer->IsAlmostFull())
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

	void AudioDecoder::PlayAudio()
	{
		// 如果存在音频流
		if (audioStream)
		{
			StartDecode();

			// 开始播放
			SDL_ResumeAudioStreamDevice(audioStream);
			// 启动线程
			//audioThreadRunning = true;
			//audioThread = std::thread(&AudioDecoder::AudioDecodeLoop, this);
		}
	}

	void AudioDecoder::StartDecode()
	{
		running = true;
		audioThread = std::thread(&AudioDecoder::AudioThread, this);
	}

	void AudioDecoder::Stop()
	{
		if (audioStream)
		{
			SDL_PauseAudioStreamDevice(audioStream);
		}

		running = false;
		if (audioThread.joinable()) audioThread.join();
	}

	void AudioDecoder::Close()
	{
		Stop();

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
	}

	double AudioDecoder::GetDuration() const
	{
		if (!m_fContext->formatContext) return 0.0;
		return m_fContext->formatContext->duration * av_q2d(AV_TIME_BASE_Q);
	}
}
