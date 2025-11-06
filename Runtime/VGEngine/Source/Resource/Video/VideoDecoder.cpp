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

    //ffmpeg -err_detect ignore_err -i yourfile.mp4 -c copy -movflags +faststart output.mp4
    bool VideoDecoder::ReadFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& videoPts, double& audioPts)
    {
        if (!formatContext || !codecContext) return false;

        while (true) {
            //H_LOG_INFO("ReadFrame");
            AVPacket* packet = av_packet_alloc();
            // 读取下一帧
            int ret = av_read_frame(formatContext, packet);
            if (ret < 0) {
                // 可能是文件结束
                return false;
            }

            // 处理音频流
            if (packet->stream_index == audioStreamIndex) {
                avcodec_send_packet(actx, packet);

                if (avcodec_receive_frame(actx, aframe) >= 0) {
                    int samples = swr_convert(swr, audioBuf, audioMaxSamples, (const uint8_t**)aframe->data, aframe->nb_samples);

                    if (samples > 0 && samples <= audioMaxSamples) {
                        int channels = 2;
                        int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                        int bytes = samples * channels * bps;
                        int bufSize = av_samples_get_buffer_size(
                            nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);
                        audioRingBuffer->Write(audioBuf[0], bufSize);
                    }
                    else if (samples > audioMaxSamples) {
                        std::cerr << "Warning: swr_convert output exceeds buffer!" << std::endl;
                    }

                }

                // 获取时间戳
                //pts = av_frame_get_best_effort_timestamp(frame) * av_q2d(timeBase);
                //pts = frame->best_effort_timestamp * av_q2d(timeBase);
                AVRational streamTimeBase = formatContext->streams[audioStreamIndex]->time_base;
                audioPts = aframe->best_effort_timestamp * av_q2d(streamTimeBase);
                videoPts = -1;

                av_packet_unref(packet);
                return true; // 继续读取视频帧
            }

            // 处理视频流
            else if (packet->stream_index == videoStreamIndex) {
                // 将包发送到解码器
                ret = avcodec_send_packet(codecContext, packet);
                if (ret < 0) {
                    std::cerr << "Error sending packet to decoder" << std::endl;
                    av_packet_unref(packet);
                    return false;
                }

                // 从解码器接收帧
                ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_packet_unref(packet);
                    continue;
                }
                else if (ret < 0) {
                    std::cerr << "Error receiving frame from decoder" << std::endl;
                    av_packet_unref(packet);
                    return false;
                }

                // 转换为RGB格式
                sws_scale(swsContext, (const uint8_t* const*)frame->data,
                    frame->linesize, 0, height,
                    frameRGB->data, frameRGB->linesize);

                // 获取时间戳
                //pts = av_frame_get_best_effort_timestamp(frame) * av_q2d(timeBase);
                //pts = frame->best_effort_timestamp * av_q2d(timeBase);
                AVRational streamTimeBase = formatContext->streams[videoStreamIndex]->time_base;
                videoPts = frame->best_effort_timestamp * av_q2d(streamTimeBase);
                audioPts = -1; // 本次返回的是视频帧

                // 返回帧数据
                frameData = frameRGB->data[0];
                linesize = frameRGB->linesize[0];
                width = this->width;
                height = this->height;

                av_packet_unref(packet);
                return true;
            }


            av_packet_unref(packet);
        }

        return true;
    }

    void VideoDecoder::DemuxLoop() {
        while (running) {
            AVPacket* pkt = av_packet_alloc();
            if (av_read_frame(formatContext, pkt) < 0) {
                demuxFinished = true;
                cond.notify_all();
                av_packet_free(&pkt);
                break;
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
    }

    void VideoDecoder::AudioDecodeLoop()
    {
        if (audioStreamIndex == -1)
            return;

        while (running) {
            AVPacket* pkt = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex);
                cond.wait(lock, [this] {
                    return !aPackets.empty() || demuxFinished;
                    });
                if (aPackets.empty() && demuxFinished) {
                    audioRingBuffer->WriteFinish();
                    break;
                }
                pkt = aPackets.front(); aPackets.pop();
            }

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

                    AVRational tb = formatContext->streams[audioStreamIndex]->time_base;
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
    }

    bool VideoDecoder::GetVideoFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& pts)
    {
        while (running) {
            AVPacket* pkt = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (vPackets.empty() && demuxFinished) return false;
                if (vPackets.empty()) continue;
                pkt = vPackets.front(); vPackets.pop();
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
    }

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
    }

    void VideoDecoder::Close()
    {
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
