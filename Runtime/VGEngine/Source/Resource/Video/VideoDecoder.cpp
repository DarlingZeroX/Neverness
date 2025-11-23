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

#include "Resource/Video/VideoDecoder.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include "Core/VFS.h"

namespace VisionGal
{
    VideoDecoder::VideoDecoder()
        : formatContext(nullptr), codecContext(nullptr),
        videoStreamIndex(-1), swsContext(nullptr),
        frame(nullptr), frameRGB(nullptr) {

		audioBuf[0] = nullptr;
		audioBuf[1] = nullptr;
    }

    VideoDecoder::~VideoDecoder()
    {
        Close();
    }

    void VideoDecoder::SetDecodeLoop(bool enable)
	{
		m_LoopPlay = enable;
	}

    bool VideoDecoder::IsDecodeLoop()
	{
		return m_LoopPlay;
	}

    AVCodecParameters* VideoDecoder::FindStream()
    {
        videoStreamIndex = -1;
        audioStreamIndex = -1;
        AVCodecParameters* codecParameters = nullptr;

        for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                codecParameters = formatContext->streams[i]->codecpar;
            }

            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
            }
        }

        return codecParameters;
    }

    bool VideoDecoder::OpenVideoDecoder(const AVCodecParameters* codecParameters)
    {
		// 查找视频解码器
		const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
		if (!codec) {
		   std::cerr << "Failed to find codec" << std::endl;
		   return false;
		}

		// 分配解码器上下文
		codecContext = avcodec_alloc_context3(codec);
		if (!codecContext) {
		   std::cerr << "Failed to allocate codec context" << std::endl;
		   return false;
		}

		// 填充解码器上下文
		if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
		   std::cerr << "Failed to copy codec parameters to codec context" << std::endl;
		   return false;
		}

		// 如果像素格式未指定，尝试强制设置常见格式
		if (codecParameters->format == AV_PIX_FMT_NONE) {
		   std::cout << "Pixel format not specified in stream, trying default formats" << std::endl;

		   // 对于H.264视频，常见格式是YUV420P
		   if (codecParameters->codec_id == AV_CODEC_ID_H264) {
		       codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		       std::cout << "Forcing H.264 pixel format to AV_PIX_FMT_YUV420P" << std::endl;
		   }
		}

		// 打开解码器
		if (avcodec_open2(codecContext, codec, nullptr) < 0) {
		   std::cerr << "Failed to open codec" << std::endl;
		   return false;
		}
      
        return true;
    }
	 
    bool VideoDecoder::Open(const std::string& filePath)
    {
        // 初始化 FFmpeg
        avformat_network_init();
        //av_register_all();

        auto& vfs = VFS::GetInstance();

        // 尝试打开文件
        auto file = vfs->OpenFile(vfspp::FileInfo(filePath), vfspp::IFile::FileMode::Read);
        if (!file)
            return false;

        // 文件是否打开
        if (file->IsOpened() == false)
            return false;

        // 用ffmpeg读取视频
        //AVFormatContext* fmt_ctx = avformat_alloc_context();
        formatContext = avformat_alloc_context();
        io_ctx = { file }; // file 是已打开的 vfspp::IFile 对象

        // 创建 AVIOContext（关键！）
        const int buffer_size = 4096; // 建议缓冲区大小
        io_buffer = static_cast<unsigned char*>(av_malloc(buffer_size));
        avio_ctx = avio_alloc_context(
            io_buffer, buffer_size,       // 缓冲区及大小
            0,                            // 只读模式
            &io_ctx,                      // 传递给回调的 opaque 对象
            static_ffmpeg_read_packet,                  // 绑定回调函数
            nullptr,                      // 无需写回调
            static_ffmpeg_seek                        // Seek 回调（可选）
        );

        // 关联到 FormatContext
        formatContext->pb = avio_ctx;

        // 增加探测深度以处理复杂或损坏的文件
        {
            //ffmpeg -err_detect ignore_err -i yourfile.mp4 -c copy -movflags +faststart output.mp4
            AVDictionary* options = nullptr;
            av_dict_set(&options, "analyzeduration", "200000000", 0); // 最大int值
            av_dict_set(&options, "probesize", "200000000", 0);       // 最大int值
            av_dict_set(&options, "movflags", "faststart", 0);   // 修复MOOV原子位置
            av_dict_set(&options, "err_detect", "ignore_err", 0); //这样在读取数据时会少报错（但如果数据真损坏还是会丢帧）。

            // 打开输入文件
            if (avformat_open_input(&formatContext, nullptr, nullptr, &options) != 0) {
                std::cerr << "Failed to open input file" << std::endl;
                av_dict_free(&options);
                return false;
            }

            // 释放选项字典
            av_dict_free(&options);
        }

        // 读取文件信息,获取流信息
        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
            std::cerr << "Failed to find stream information" << std::endl;
            return false;
        }

        // 打印文件信息（可选）
        //av_dump_format(formatContext, 0, filePath.c_str(), 0);

        // 查找视频流和音频流
        AVCodecParameters* codecParameters = FindStream();

        // 音频解码器
        if (audioStreamIndex != -1)
        {
            // 音频
            actx = avcodec_alloc_context3(avcodec_find_decoder(formatContext->streams[audioStreamIndex]->codecpar->codec_id));
            avcodec_parameters_to_context(actx, formatContext->streams[audioStreamIndex]->codecpar);
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

            audioRingBuffer = CreateRef<AudioRingBuffer>(8 * 1024 * 1024);

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

        // 视频
        if (videoStreamIndex == -1) {
            std::cerr << "Failed to find video stream" << std::endl;
            return false;
        }

        // timeBase 直接从 stream 拿，避免多次读包跳帧
        timeBase = formatContext->streams[videoStreamIndex]->time_base;

        // 打印调试
        //std::cout << "Video stream timeBase: " << timeBase.num << "/" << timeBase.den
        //    << " (" << av_q2d(timeBase) << " sec/unit)" << std::endl;

        // 视频
        OpenVideoDecoder(codecParameters);
        //{
        //    // 查找视频解码器
        //    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        //    if (!codec) {
        //        std::cerr << "Failed to find codec" << std::endl;
        //        return false;
        //    }
        //
        //    // 分配解码器上下文
        //    codecContext = avcodec_alloc_context3(codec);
        //    if (!codecContext) {
        //        std::cerr << "Failed to allocate codec context" << std::endl;
        //        return false;
        //    }
        //
        //    // 填充解码器上下文
        //    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        //        std::cerr << "Failed to copy codec parameters to codec context" << std::endl;
        //        return false;
        //    }
        //
        //    // 如果像素格式未指定，尝试强制设置常见格式
        //    if (codecParameters->format == AV_PIX_FMT_NONE) {
        //        std::cout << "Pixel format not specified in stream, trying default formats" << std::endl;
        //
        //        // 对于H.264视频，常见格式是YUV420P
        //        if (codecParameters->codec_id == AV_CODEC_ID_H264) {
        //            codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
        //            std::cout << "Forcing H.264 pixel format to AV_PIX_FMT_YUV420P" << std::endl;
        //        }
        //    }
        //
        //    // 打开解码器
        //    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        //        std::cerr << "Failed to open codec" << std::endl;
        //        return false;
        //    }
        //}

        // 再次检查像素格式
        if (codecContext->pix_fmt == AV_PIX_FMT_NONE) {
            std::cerr << "Decoder still didn't set a pixel format" << std::endl;

            // 尝试从编解码器参数获取
            if (codecParameters->format != AV_PIX_FMT_NONE) {
                codecContext->pix_fmt = static_cast<AVPixelFormat>(codecParameters->format);
                std::cout << "Using pixel format from codec parameters: "
                    << av_get_pix_fmt_name(codecContext->pix_fmt) << std::endl;
            }
            else {
                // 最后尝试设置一个安全的默认值
                codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
                std::cout << "Forcing default pixel format to AV_PIX_FMT_YUV420P" << std::endl;
            }
        }

        // 获取视频宽高
        width = codecContext->width;
        height = codecContext->height;

        // 分配帧和包
        frame = av_frame_alloc();
        aframe = av_frame_alloc();
        frameRGB = av_frame_alloc();
        if (!frame || !frameRGB) {
            std::cerr << "Failed to allocate frame" << std::endl;
            return false;
        }

        // 确定RGB格式和分配缓冲区
        numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
        buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

        av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer,
            AV_PIX_FMT_RGBA, width, height, 1);

        // 初始化SWS上下文用于像素格式转换
        swsContext = sws_getContext(width, height, codecContext->pix_fmt,
            width, height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!swsContext) {
            std::cerr << "Failed to initialize sws context" << std::endl;
            return false;
        }

        // 初始化包
        //packet = av_packet_alloc();
        //if (!packet) {
        //    std::cerr << "Failed to allocate packet" << std::endl;
        //    return false;
        //}
        //if (av_read_frame(formatContext, packet) >= 0) {
        //    if (packet->stream_index == videoStreamIndex) {
        //        timeBase = formatContext->streams[videoStreamIndex]->time_base;
        //        av_packet_unref(packet);
        //    }
        //}
        //else
        //{
        //    // 尝试seek到0再读
        //    av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
        //    if (av_read_frame(formatContext, packet) < 0) {
        //        std::cerr << "Still failed to read frame after seek" << std::endl;
        //        return false;
        //    }
        //}
        //

        return true;
    }

	// 修改目标：修复在开启循环播放(m_EnableDecodeLoop)时，回到文件头重新循环导致崩溃的问题
	// 设计思路（伪代码，详细说明）：
	// 1. 当 av_read_frame 返回 < 0（通常为文件结束或读取错误）并且开启循环播放时：
	//    a) 先释放本次循环分配的 pkt（防止内存泄漏）
	//    b) 等待队列缩小（使用 wait_for 防止永久阻塞），保留超时日志以便排查
	//    c) 在持有 mutex 的情况下，清空并释放 vPackets 和 aPackets 队列中的所有 AVPacket*
	//       - 这是关键：旧的 packet 在 seek 后可能与新的流状态不一致，保留会导致后续解码/释放混乱导致崩溃
	//    d) 执行 seek（优先 av_seek_frame，失败时尝试 avformat_seek_file）
	//    e) 刷新解码器内部缓存（avcodec_flush_buffers），重置时钟与标志位
	//    f) notify_all 唤醒消费者/生产者线程，短暂 sleep 后继续 DemuxLoop
	// 2. 当不开启循环播放或停止时：标记 demuxFinished = true、唤醒并退出循环（同时确保已释放 pkt）
	// 3. 其它逻辑保持不变（线程安全：所有对队列的修改都在 mutex 保护下执行）
	// 说明：主要修复点为清空队列并释放包以及确保分支中分配的 pkt 被释放，避免 stale packet 导致的崩溃或双重释放。

	void VideoDecoder::DemuxLoop() 
	{
		const int MAX_QUEUE_SIZE = 200; // 最大缓存包数量
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200); // 兜底超时

		while (running) 
		{
			AVPacket* pkt = av_packet_alloc();
			int ret = av_read_frame(formatContext, pkt);

			// 读取结束或出错
			if (ret < 0) 
			{
				// 先释放本次分配的 pkt
				av_packet_free(&pkt);

				if (m_LoopPlay && running) 
				{
					// 等待队列消化到可接受范围，避免立即清空导致竞争（带超时）
					{
						std::unique_lock<std::mutex> lock(mutex);
						//if (!cond.wait_for(lock, WAIT_TIMEOUT, [&]() {
						//	//return (vPackets.size() < 10 && aPackets.size() < 10) || !running;
						//	return (vPackets.empty() && aPackets.empty()) || !running;
						//	})) {
						//	std::cerr << "[Demux] wait_for timeout while waiting for queues to shrink (v=" << vPackets.size()
						//		<< ", a=" << aPackets.size() << ")\n";
						//}

						// 等级队列清空
						cond.wait(lock, [&]() {
							//return (vPackets.size() < 10 && aPackets.size() < 10) || !running;
							return (vPackets.empty() && aPackets.empty()) || !running;
							});

						// 关键：清空并释放队列中残留的 AVPacket*，避免在 seek 后使用到 stale packet 导致崩溃
						//while (!vPackets.empty()) {
						//	AVPacket* p = vPackets.front();
						//	vPackets.pop();
						//	if (p) av_packet_free(&p);
						//}
						//while (!aPackets.empty()) {
						//	AVPacket* p = aPackets.front();
						//	aPackets.pop();
						//	if (p) av_packet_free(&p);
						//}
					}

					// 回到文件头（优先全局 seek）
					if (av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD) < 0) 
					{
						std::cerr << "[Demux] Warning: av_seek_frame failed, trying avformat_seek_file\n";
						avformat_seek_file(formatContext, -1, INT64_MIN, 0, INT64_MAX, 0);
					}

					// 刷新解码器状态，重置时钟
					if (actx) avcodec_flush_buffers(actx);
					if (codecContext) avcodec_flush_buffers(codecContext);

					audioClock = 0.0;

					{
						std::lock_guard<std::mutex> lock(mutex);
						demuxFinished = false;
					}

					// 通知所有等待线程（消费或生产者都可以）
					cond.notify_all();

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
				else 
				{
					// 正常结束
					{
						std::lock_guard<std::mutex> lock(mutex);
						demuxFinished = true;
					}
					cond.notify_all();
					break;
				}
			}

			// 控制队列大小（防止爆内存） - 使用 wait_for 兜底
			{
				std::unique_lock<std::mutex> lock(mutex);
				if (!cond.wait_for(lock, WAIT_TIMEOUT, [&]() {
					return (vPackets.size() < MAX_QUEUE_SIZE || aPackets.size() < MAX_QUEUE_SIZE) || !running;
					})) {
					std::cerr << "[Demux] wait_for timeout while waiting for free queue space (v=" << vPackets.size()
						<< ", a=" << aPackets.size() << ")\n";
				}
			}

			if (!running) {
				av_packet_free(&pkt);
				break;
			}

			// 按流类型分发
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (pkt->stream_index == videoStreamIndex) {
					vPackets.push(pkt);
				}
				else if (pkt->stream_index == audioStreamIndex) {
					aPackets.push(pkt);
				}
				else {
					av_packet_free(&pkt);
				}
			}

			// 通知消费者（有新包了）
			cond.notify_all();
		}

		// 收尾清理
		{
			std::lock_guard<std::mutex> lock(mutex);
			demuxFinished = true;
		}
		cond.notify_all();

		std::cout << "[Demux] DemuxLoop finished.\n";
	}

	/*
    void VideoDecoder::DemuxLoop() {
        while (running) {
            AVPacket* pkt = av_packet_alloc();
            if (av_read_frame(formatContext, pkt) < 0) {

				// 到达文件结尾
				if (m_EnableDecodeLoop && running)
				{
					while (true)
					{
						//std::lock_guard<std::mutex> lock(mutex);
						if (vPackets.size() > 100 || aPackets.size() > 100)
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						else
							break;
					}

					// 尝试回到文件头并继续解码
					// 对音频流使用时间戳 0，向后搜索
					int ret = av_seek_frame(formatContext, audioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					if (ret < 0)
					{
						// 如果按流索引失败，尝试全局 seek
						av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD);
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
					// 结束

					demuxFinished = true;
					cond.notify_all();
					av_packet_free(&pkt);
					break;
				}
            }
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (pkt->stream_index == videoStreamIndex)
                    vPackets.push(pkt);
                else if (pkt->stream_index == audioStreamIndex)
                    aPackets.push(pkt);
                else
                    av_packet_free(&pkt);
            }
            cond.notify_all();
        }
    }*/

    void VideoDecoder::AudioDecodeLoop()
    {
        if (audioStreamIndex == -1)
            return;

        while (running) {
            //AVPacket* pkt = nullptr;
			AVPacket pktLocal;
            {
                std::unique_lock<std::mutex> lock(mutex);
                cond.wait(lock, [this] {
                    return !aPackets.empty() || demuxFinished;
                    });
                if (aPackets.empty() && demuxFinished) {
                    audioRingBuffer->WriteFinish();
                    break;
                }

				av_packet_ref(&pktLocal, aPackets.front());
                //pkt = aPackets.front();
				aPackets.pop();
            }

            avcodec_send_packet(actx, &pktLocal);
            //av_packet_free(&pkt);
			av_packet_unref(&pktLocal);
            while (avcodec_receive_frame(actx, aframe) >= 0) {
                int samples = swr_convert(swr, audioBuf, audioMaxSamples, (const uint8_t**)aframe->data, aframe->nb_samples);

				AVRational tb = formatContext->streams[audioStreamIndex]->time_base;
				audioClock = aframe->best_effort_timestamp * av_q2d(tb);

                if (samples > 0) {
                    int channels = 2;
                    int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                    int bytes = samples * channels * bps;
                    int bufSize = av_samples_get_buffer_size(
                        nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);
                    audioRingBuffer->Write(audioBuf[0], bufSize);
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
    }

    bool VideoDecoder::VideoDecodeLoop()
    {
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200);

		uint8_t* frameData = nullptr;
		int linesize = 0;
		int width = 0;
		int height = 0;
		double pts = 0.f;

		while (running) {
			AVPacket pktLocal;
			bool gotPacket = false;

			{
				std::unique_lock<std::mutex> lock(mutex);

				// 等待 packet
				{
					//std::unique_lock<std::mutex> lock(mutex);
					if (!cond.wait_for(lock, WAIT_TIMEOUT, [&]() { return !vPackets.empty() || demuxFinished || !running; }))
						continue;
					if (!vPackets.empty()) {
						av_packet_ref(&pktLocal, vPackets.front());
						gotPacket = true;
					}
					//else if (demuxFinished && m_LoopPlay) {
					//	avcodec_flush_buffers(codecContext);
					//	av_frame_unref(frame);
					//	av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					//	demuxFinished = false;
					//	continue;
					//}
					else if (demuxFinished)
						return false;
				}

				// 发送 packet
				if (gotPacket) {
					//av_packet_rescale_ts(&pktLocal,
					//	formatContext->streams[videoStreamIndex]->time_base,
					//	codecContext->time_base);

					int ret = avcodec_send_packet(codecContext, &pktLocal);
					av_packet_unref(&pktLocal);

					if (ret < 0 && ret != AVERROR(EAGAIN)) return false;

					if (ret == AVERROR(EAGAIN)) {
						// 输出队列满：尝试接收并释放一帧，让 decoder 有空间，然后重试（不 pop pkg）
						AVFrame* tmp = av_frame_alloc();
						ret = avcodec_receive_frame(codecContext, tmp);
						if (ret >= 0) {
							// Got a frame: convert并返回（注意这并不改变 pkt 在队列里的位置）
							sws_scale(swsContext, (const uint8_t* const*)tmp->data,
								tmp->linesize, 0, height,
								frameRGB->data, frameRGB->linesize);

							AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
							pts = tmp->best_effort_timestamp * av_q2d(tb);
							frameData = frameRGB->data[0];
							linesize = frameRGB->linesize[0];
							width = this->width;
							height = this->height;

							av_frame_free(&tmp);
							return true;
						}
						// 若没有拿到 frame（比如 EAGAIN 再次），释放 tmp 并重试 send（注意 pkt 仍在队列）
						av_frame_free(&tmp);
						// 小睡避免 busy-loop
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;
					}

					// 弹出队列
					{
						//std::lock_guard<std::mutex> lock(mutex);
						if (!vPackets.empty()) vPackets.pop();
						cond.notify_all();
					}
				}
			}

			// 接收 frame
			while (true) {
				//std::cout << vPackets.size() << std::endl;
				//av_frame_unref(frame);
				int ret = avcodec_receive_frame(codecContext, frame);
				if (ret == AVERROR(EAGAIN)) break;
				if (ret == AVERROR_EOF) {
					//if (m_LoopPlay) {
					//	avcodec_flush_buffers(codecContext);
					//	av_frame_unref(frame);
					//	av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					//	demuxFinished = false;
					//	break;
					//}
					return false;
				}
				if (ret < 0) return false;

				sws_scale(swsContext, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
				AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
				pts = frame->best_effort_timestamp * av_q2d(tb);

				// 尝试修复时间
				//if (frame->best_effort_timestamp != AV_NOPTS_VALUE) {
				//	AVRational tb = codecContext->time_base; // 用 codec 时间基
				//	pts = frame->best_effort_timestamp * av_q2d(tb);
				//}
				//std::cout << "frame->best_effort_timestamp: " << frame->best_effort_timestamp << " ,pts: " << pts << std::endl;

				frameData = frameRGB->data[0];
				linesize = frameRGB->linesize[0];
				//widthOut = width;
				//heightOut = height;

				// 数据回调
				if (OnVideoDataUpdate)
					OnVideoDataUpdate(frameData, linesize, width, height, pts);
				//return true;
			}

			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		return false;
    }


    bool VideoDecoder::GetVideoFrame(uint8_t*& frameData, int& linesize, int& widthOut, int& heightOut, double& pts)
	{
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200);

		while (running) {
			AVPacket pktLocal;
			bool gotPacket = false;

			{
				std::unique_lock<std::mutex> lock(mutex);

				// 等待 packet
				{
					//std::unique_lock<std::mutex> lock(mutex);
					if (!cond.wait_for(lock, WAIT_TIMEOUT, [&]() { return !vPackets.empty() || demuxFinished || !running; }))
						continue;
					if (!vPackets.empty()) {
						av_packet_ref(&pktLocal, vPackets.front());
						gotPacket = true;
					}
					//else if (demuxFinished && m_LoopPlay) {
					//	avcodec_flush_buffers(codecContext);
					//	av_frame_unref(frame);
					//	av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					//	demuxFinished = false;
					//	continue;
					//}
					else if (demuxFinished)
						return false;
				}

				// 发送 packet
				if (gotPacket) {
					av_packet_rescale_ts(&pktLocal,
						formatContext->streams[videoStreamIndex]->time_base,
						codecContext->time_base);

					int ret = avcodec_send_packet(codecContext, &pktLocal);
					av_packet_unref(&pktLocal);

					if (ret < 0 && ret != AVERROR(EAGAIN)) return false;

					if (ret == AVERROR(EAGAIN)) {
						// 输出队列满：尝试接收并释放一帧，让 decoder 有空间，然后重试（不 pop pkg）
						AVFrame* tmp = av_frame_alloc();
						ret = avcodec_receive_frame(codecContext, tmp);
						if (ret >= 0) {
							// Got a frame: convert并返回（注意这并不改变 pkt 在队列里的位置）
							sws_scale(swsContext, (const uint8_t* const*)tmp->data,
								tmp->linesize, 0, height,
								frameRGB->data, frameRGB->linesize);

							AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
							pts = tmp->best_effort_timestamp * av_q2d(tb);
							frameData = frameRGB->data[0];
							linesize = frameRGB->linesize[0];
							width = this->width;
							height = this->height;

							av_frame_free(&tmp);
							return true;
						}
						// 若没有拿到 frame（比如 EAGAIN 再次），释放 tmp 并重试 send（注意 pkt 仍在队列）
						av_frame_free(&tmp);
						// 小睡避免 busy-loop
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;
					}

					// 弹出队列
					{
						//std::lock_guard<std::mutex> lock(mutex);
						if (!vPackets.empty()) vPackets.pop();
						cond.notify_all();
					}
				}
			}

			// 接收 frame
			while (true) {
				//std::cout << vPackets.size() << std::endl;
				av_frame_unref(frame);
				int ret = avcodec_receive_frame(codecContext, frame);
				if (ret == AVERROR(EAGAIN)) break;
				if (ret == AVERROR_EOF) {
					//if (m_LoopPlay) {
					//	avcodec_flush_buffers(codecContext);
					//	av_frame_unref(frame);
					//	av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
					//	demuxFinished = false;
					//	break;
					//}
					return false;
				}
				if (ret < 0) return false;

				sws_scale(swsContext, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
				AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
				pts = frame->best_effort_timestamp * av_q2d(tb);

				frameData = frameRGB->data[0];
				linesize = frameRGB->linesize[0];
				widthOut = width;
				heightOut = height;
				return true;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		return false;
	}
	/*
	bool VideoDecoder::GetVideoFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& pts)
	{
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200);

		while (running) {
			AVPacket* pkt = nullptr;

			// 1) 等待有 packet 可用 或 demuxFinished
			{
				std::unique_lock<std::mutex> lock(mutex);
				if (!cond.wait_for(lock, WAIT_TIMEOUT, [&]() { return !vPackets.empty() || demuxFinished || !running; })) {
					// 超时继续检查状态
					if (!running) return false;
				}

				if (vPackets.empty()) {
					if (demuxFinished) return false;
					continue;
				}

				// 只 **peek** 队首，不 pop（直到 send 成功）
				pkt = vPackets.front();
				// 我们将在 send 成功后再 pop（并 notify_all）
			}

			// 2) 在发送前把 packet 的时间基转换到解码器需要的 time_base
			//    注意：formatContext->streams[videoStreamIndex]->time_base -> codecContext->time_base
			av_packet_rescale_ts(pkt,
				formatContext->streams[videoStreamIndex]->time_base,
				codecContext->time_base);

			// 3) 发送 packet 到解码器
			int ret = avcodec_send_packet(codecContext, pkt);
			if (ret == AVERROR(EAGAIN)) {
				// 输出队列满：尝试接收并释放一帧，让 decoder 有空间，然后重试（不 pop pkg）
				AVFrame* tmp = av_frame_alloc();
				ret = avcodec_receive_frame(codecContext, tmp);
				if (ret >= 0) {
					// Got a frame: convert并返回（注意这并不改变 pkt 在队列里的位置）
					sws_scale(swsContext, (const uint8_t* const*)tmp->data,
						tmp->linesize, 0, height,
						frameRGB->data, frameRGB->linesize);

					AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
					pts = tmp->best_effort_timestamp * av_q2d(tb);
					frameData = frameRGB->data[0];
					linesize = frameRGB->linesize[0];
					width = this->width;
					height = this->height;

					av_frame_free(&tmp);
					return true;
				}
				// 若没有拿到 frame（比如 EAGAIN 再次），释放 tmp 并重试 send（注意 pkt 仍在队列）
				av_frame_free(&tmp);
				// 小睡避免 busy-loop
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else if (ret < 0) {
				std::cerr << "[VideoDecoder] avcodec_send_packet error: " << ret << std::endl;
				// 如果 send 出错（并非 EAGAIN），说明这个 packet 有问题，丢弃它
				// 在持锁内 pop 并释放它，然后 notify
				{
					std::lock_guard<std::mutex> lock(mutex);
					if (!vPackets.empty() && vPackets.front() == pkt) {
						vPackets.pop();
					}
				}
				av_packet_free(&pkt);
				cond.notify_all();
				return false;
			}

			// 发送成功 —— 现在我们可以安全地从队列 pop 这个 packet 并 notify demux
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (!vPackets.empty() && vPackets.front() == pkt) {
					vPackets.pop();
				} else {
					// 极端情况：队列不一致（不应发生），仍然继续
				}
			}
			// 已经从队列移除，释放 packet 内存
			av_packet_free(&pkt);

			// 通知 demux 生产者（队列空间已释放）
			cond.notify_all();

			// 4) 读取输出帧（可能有多个帧可取）
			while (true) {
				std::cout << vPackets.size() << std::endl;
				ret = avcodec_receive_frame(codecContext, frame);
				if (ret == AVERROR(EAGAIN)) {
					// 没有可用的帧（解码器需要更多 packet）
					break;
				}
				else if (ret == AVERROR_EOF) {
					// 到达解码 EOF
					break;
				}
				else if (ret < 0) {
					std::cerr << "[VideoDecoder] avcodec_receive_frame error: " << ret << std::endl;
					return false;
				}

				// 成功收到一帧 -> 转成 RGB 返回
				sws_scale(swsContext, (const uint8_t* const*)frame->data,
					frame->linesize, 0, height,
					frameRGB->data, frameRGB->linesize);

				AVRational tb = formatContext->streams[videoStreamIndex]->time_base;
				pts = frame->best_effort_timestamp * av_q2d(tb);

				frameData = frameRGB->data[0];
				linesize = frameRGB->linesize[0];
				width = this->width;
				height = this->height;

				// 注意：frame 是由 av_frame_unref/av_frame_free 在其他地方管理（如果是成员frame，别 free）
				return true;
			}

			// 如果没有拿到帧（例如 receive 返回 EAGAIN），继续循环等待新的 packet/send
		}

		return false;
	}*/


	/*
    bool VideoDecoder::GetVideoFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& pts)
    {
        while (running) {
            AVPacket* pkt = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex);
				if (vPackets.empty() && demuxFinished)
				{
					return false;
				}
                if (vPackets.empty()) continue;
                pkt = vPackets.front(); vPackets.pop();
				std::cout << vPackets.size() << std::endl;
            }

            // 将包发送到解码器
            int ret = avcodec_send_packet(codecContext, pkt);
            if (ret < 0) {
                std::cerr << "Error sending packet to decoder" << std::endl;
                av_packet_unref(pkt);
                return false;
            }

            // 从解码器接收帧
            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_packet_unref(pkt);
                continue;
            }
            else if (ret < 0) {
                std::cerr << "Error receiving frame from decoder" << std::endl;
                av_packet_unref(pkt);
                return false;
            }

            // 转换为RGB格式
            sws_scale(swsContext, (const uint8_t* const*)frame->data,
                frame->linesize, 0, height,
                frameRGB->data, frameRGB->linesize);

            // 获取时间戳
            AVRational streamTimeBase = formatContext->streams[videoStreamIndex]->time_base;
            pts = frame->best_effort_timestamp * av_q2d(streamTimeBase);
    
            // 返回帧数据
            frameData = frameRGB->data[0];
            linesize = frameRGB->linesize[0];
            width = this->width;
            height = this->height;

            av_packet_unref(pkt);
            return true;
        }
        return false;
    }*/

    void VideoDecoder::PlayAudio()
    {
        // 如果存在音频流
        if (audioStream)
        {
            // 开始播放
            SDL_ResumeAudioStreamDevice(audioStream);
            // 启动线程
            //audioThreadRunning = true;
            //audioThread = std::thread(&VideoDecoder::AudioDecodeLoop, this);
        }
    }

    void VideoDecoder::Start()
    {
        running = true;
        demuxThread = std::thread(&VideoDecoder::DemuxLoop, this);
        audioThread = std::thread(&VideoDecoder::AudioDecodeLoop, this);
        videoThread = std::thread(&VideoDecoder::VideoDecodeLoop, this);
    }

    void VideoDecoder::Stop()
    {
        if (audioStream)
        {
            SDL_PauseAudioStreamDevice(audioStream);
        }

        running = false;

        // 确保解码线程能正常退出
        {
            std::lock_guard<std::mutex> lock(mutex);
            demuxFinished = true;
        }
        cond.notify_all();

        if (demuxThread.joinable()) demuxThread.join();

        /*
         
         为什么 audioThread 卡住在 join？
		因为：

		audioThread 永远停在：

		cond.wait(lock, [this] {
		    return !aPackets.empty() || demuxFinished;
		});
		running 其实不影响这个条件，wait 也不会醒。

		所以它无法跳出 while 循环，线程永远在阻塞状态，join 就会卡死。

		如何正确解决
		在 Stop() 中，除了 running = false，还需要保证：

		demuxFinished = true

		并 cond.notify_all(); 唤醒线程去检查条件 
         */
        if (audioThread.joinable()) audioThread.join();
        if (videoThread.joinable()) videoThread.join();
    }

    void VideoDecoder::Close()
    {
		running = false;
		Stop();

        // 视频
        if (formatContext) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }

        if (codecContext) {
            avcodec_free_context(&codecContext);
            codecContext = nullptr;
        }

        if (frame) {
            av_frame_free(&frame);
            frame = nullptr;
        }

        if (frameRGB) {
            av_frame_free(&frameRGB);
            frameRGB = nullptr;
        }

        if (swsContext) {
            sws_freeContext(swsContext);
            swsContext = nullptr;
        }

        if (buffer) {
            av_free(buffer);
            buffer = nullptr;
        }

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

		//if (audioBuf[0])
		//{
		//	av_freep(audioBuf[0]);
		//}
		//
		//if (audioBuf[1])
		//{
		//	av_freep(audioBuf[1]);
		//}

		//if (io_buffer)
		//{
		//	av_free(io_buffer);
		//	io_buffer = nullptr;
		//}

		if (avio_ctx)
		{
			avio_context_free(&avio_ctx);
			avio_ctx = nullptr; // 释放 AVIOContext
		}
    }

    double VideoDecoder::GetDuration() const
    {
        if (!formatContext) return 0.0;
        return formatContext->duration * av_q2d(AV_TIME_BASE_Q);
    }
}
