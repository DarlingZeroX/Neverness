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

#include "Video/FVideoDecoder.h"
//#include "Core/VFS.h"
extern "C" {
#include <libavutil/imgutils.h>
}

namespace NN::Core {

	FVideoDecoder::FVideoDecoder()
	{
	}

	FVideoDecoder::~FVideoDecoder()
	{
	}

	bool FVideoDecoder::InitializeAudio(FfmpegContext& context)
	{
		// 查找音频流
		m_AudioStreamIndex = context.FindAudioStreamIndex();
		if (m_AudioStreamIndex == -1)				// 音频解码器
			return false;

		// 音频Codec上下文
		m_AudioCodecContext = MakeRef<FfmpegAVCodecContext>(*context.GetFormatContext(), m_AudioStreamIndex);

		// 假设 actx 是已经 open2 的 AVCodecContext*
		FfmpegAVChannelLayout in_ch_layout(m_AudioCodecContext->GetCHLayout());
		FfmpegAVChannelLayout out_ch_layout(2);
		m_AudioSwrContext = FfmpegSwrContext::Create(in_ch_layout, out_ch_layout, *m_AudioCodecContext);

		// 创建音频数据缓冲区
		m_AudioRingBuffer = MakeRef<AudioRingBuffer>(2 * 1024 * 1024);

		// 创建Ffmpeg音频缓冲区
		m_AudioMaxSamples = av_rescale_rnd(4096, 44100, m_AudioCodecContext->GetSampleRate(), AV_ROUND_UP);
		av_samples_alloc(&m_AudioBuf, nullptr, 2, m_AudioMaxSamples, AV_SAMPLE_FMT_S16, 0);

		// 创建音频帧
		m_AudioFrame = MakeRef<FfmpegAVFrame>();

		return true;
	}

	double FVideoDecoder::GetDuration() const
	{
		auto* formatCtx = m_FContext->GetFormatContext();
		if (formatCtx->GetDuration() != AV_NOPTS_VALUE)
			return (double)formatCtx->GetDuration() / AV_TIME_BASE;

		if (m_VideoStreamIndex >= 0)
		{
			AVStream* st = formatCtx->GetStream(m_VideoStreamIndex);
			if (st->duration != AV_NOPTS_VALUE)
				return static_cast<double>(st->duration) * st->time_base.num / st->time_base.den;
		}

		if (m_AudioStreamIndex >= 0)
		{
			AVStream* st = formatCtx->GetStream(m_AudioStreamIndex);
			if (st->duration != AV_NOPTS_VALUE)
				return static_cast<double>(st->duration) * st->time_base.num / st->time_base.den;
		}

		return 0.f;
	}

	bool FVideoDecoder::HasAudioStream() const
	{
		return m_AudioStreamIndex != -1;
	}

	bool FVideoDecoder::InitializeVideo(FfmpegContext& context)
	{
		// 查找视频流
		m_VideoStreamIndex = m_FContext->FindVideoStreamIndex();
		if (m_VideoStreamIndex == -1)
			return false;

		// 获取TimeBase
		auto* formatCtx = m_FContext->GetFormatContext();
		// timeBase 直接从 stream 拿，避免多次读包跳帧
		m_TimeBase = formatCtx->GetStream(m_VideoStreamIndex)->time_base;

		// 视频Codec上下文
		m_VideoCodecContext = MakeRef<FfmpegAVCodecContext>(*context.GetFormatContext(), m_VideoStreamIndex, true);

		// 再设置一下视频格式
		AVCodecParameters* videoCodeParam = formatCtx->GetStream(m_VideoStreamIndex)->codecpar;;
		m_VideoCodecContext->SetPixelFormat(videoCodeParam);

		// 创建视频帧
		m_VideoFrame = MakeRef<FfmpegAVFrame>();
		m_VideoRGBFrame = MakeRef<FfmpegAVFrame>();

		// 确定RGB格式和分配缓冲区
		m_VideoWidth = m_VideoCodecContext->GetVideoWidth();
		m_VideoHeight = m_VideoCodecContext->GetVideoHeight();
		auto numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_VideoWidth, m_VideoHeight, 1);
		m_RGBBuffer = FfmpegBuffer::New(numBytes * sizeof(uint8_t));

		//填充Frame
		m_VideoRGBFrame->VideoImageFillArrays(*m_RGBBuffer, AV_PIX_FMT_RGBA, m_VideoWidth, m_VideoHeight, 1);

		m_VideoSwsContext = FfmpegSwsContext::Create(
			m_VideoWidth, m_VideoHeight, m_VideoCodecContext->GetPixelFormat(),
			m_VideoWidth, m_VideoHeight, AV_PIX_FMT_RGBA,
			SWS_BILINEAR, nullptr, nullptr, nullptr
		);
	}

	void FVideoDecoder::OnFinishedDecode()
	{
		m_IsRunning = false;
		m_IsDemuxFinished = true;
		m_IsFinishedDecode = true;
	}

	bool FVideoDecoder::Open(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath)
	{
		avformat_network_init();

		m_FContext = FfmpegContext::Create(vfs, filePath);

		if (m_FContext == nullptr)
			return false;

		// 初始化音频
		InitializeAudio(*m_FContext);

		// 初始化视频
		if (InitializeVideo(*m_FContext) == false)
			return false;

		return true;
	}

	bool FVideoDecoder::StartDecode()
	{
		StopDecode();

		m_IsRunning = true;
		m_IsDemuxFinished = false;
		m_IsFinishedDecode = false;

		m_DemuxThread = std::thread(&FVideoDecoder::DemuxLoop, this);
		if (HasAudioStream())
			m_AudioThread = std::thread(&FVideoDecoder::AudioDecodeLoop, this);
		m_VideoThread = std::thread(&FVideoDecoder::VideoDecodeLoop, this);

		return true;
	}

	bool FVideoDecoder::StopDecode()
	{
		m_IsRunning = false;
		m_IsDemuxFinished = true;

		// 确保解码线程能正常退出
		m_Condition.notify_all();

		// 等待线程
		if (m_DemuxThread.joinable()) m_DemuxThread.join();
		if (HasAudioStream())
		{
			if (m_AudioThread.joinable()) m_AudioThread.join();
		}
		if (m_VideoThread.joinable()) m_VideoThread.join();

		return true;
	}

	bool FVideoDecoder::Seek(double seconds)
	{
		if (!m_FContext)
			return false;

		//bool isPause = m_IsPauseDecode;
		PauseDecode(true);

		// 先对线程进行加锁，防止与解码线程访问冲突
		{
			std::unique_lock lock(m_Mutex);

			auto* formatCtx = m_FContext->GetFormatContext();
			int64_t timestamp = static_cast<int64_t>(seconds / av_q2d(formatCtx->GetStream(m_VideoStreamIndex)->time_base));

			// 尝试回到文件头并继续解码
			// 1. 对音频流使用时间戳 0，向后搜索
			int ret = formatCtx->SeekFrame(m_VideoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
			if (ret < 0)
			{
				// 如果按流索引失败，尝试全局 seek
				ret = formatCtx->SeekFrame(-1, timestamp, AVSEEK_FLAG_BACKWARD);
				return ret >= 0;
			}

			// 2. 清空 RingBuffer
			if (HasAudioStream())
			{
				//int ret = formatCtx->SeekFrame(m_AudioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
				auto ring = GetAudioBuffer();
				ring->Reset();
			}
			
			// 刷新解码器状态，重置时钟
			if (m_AudioCodecContext) m_AudioCodecContext->FlushBuffers();
			if (m_VideoCodecContext) m_VideoCodecContext->FlushBuffers();

			// 清空包队列
			while (m_AudioPacketQueue.empty() == false)
				m_AudioPacketQueue.pop();
			while (m_VideoPacketQueue.empty() == false)
				m_VideoPacketQueue.pop();
		}

		PauseDecode(false); 
		m_VideoClock = seconds;
		//if (isPause == false)
		//RestoreDecode();
		//
		//m_IsDemuxFinished = false;
		//m_IsFinishedDecode = false;

		return true;
	}

	bool FVideoDecoder::IsRunningDecode() const
	{
		return m_IsRunning == true;
	}

	void FVideoDecoder::DemuxLoop()
	{
		const int MAX_QUEUE_SIZE = 200; // 最大缓存包数量
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200); // 兜底超时

		while (IsRunningDecode())
		{
			// 暂停
			if (m_IsPauseDecode) {
				if (IsRunningDecode() == false)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			FfmpegAVFormatContext* formatContext = m_FContext->GetFormatContext();

			Ref<FfmpegAVPacket> pkt = MakeRef<FfmpegAVPacket>();
			int ret = -1;
			{
				std::unique_lock lock(m_Mutex);
				ret = formatContext->ReadFrame(*pkt);
			}
			// 读取结束或出错
			if (ret < 0)
			{
				//if (m_EnableDecodeLoop && IsRunningDecode())
				//{
				//	// 等待队列消化到可接受范围，避免立即清空导致竞争（带超时）
				//	{
				//		std::unique_lock<std::mutex> lock(m_Mutex);
				//
				//		// 等级队列清空
				//		m_Condition.wait(lock, [&]() {
				//			return (m_VideoPacketQueue.empty() && m_AudioPacketQueue.empty()) || !m_IsRunning;
				//			});
				//	}
				//
				//	// 回到文件头（优先全局 seek）
				//	Seek(0);
				//	m_IsDemuxFinished = false;
				//
				//	// 通知所有等待线程（消费或生产者都可以）
				//	m_Condition.notify_all();
				//
				//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
				//	continue;
				//}
				//else
				//{
				// 正常结束
				m_IsDemuxFinished = true;
				m_Condition.notify_all();
				break;
				//}
			}

			// 控制队列大小（防止爆内存） - 使用 wait_for 兜底
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				if (!m_Condition.wait_for(lock, WAIT_TIMEOUT, [&]() {
					return (m_VideoPacketQueue.size() < MAX_QUEUE_SIZE || m_AudioPacketQueue.size() < MAX_QUEUE_SIZE) || !m_IsRunning;
					})) {
					std::cerr << "[Demux] wait_for timeout while waiting for free queue space (v=" << m_VideoPacketQueue.size()
						<< ", a=" << m_AudioPacketQueue.size() << ")\n";
				}
			}

			if (!m_IsRunning) {
				break;
			}

			// 按流类型分发
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				if (pkt->GetStreamIndex() == m_VideoStreamIndex) {
					m_VideoPacketQueue.push(pkt);
				}
				else if (pkt->GetStreamIndex() == m_AudioStreamIndex) {
					m_AudioPacketQueue.push(pkt);
				}
			}

			// 通知消费者（有新包了）
			m_Condition.notify_all();
		}

		// 收尾清理
		m_IsDemuxFinished = true;
		m_Condition.notify_all();

		std::cout << "[Demux] DemuxLoop finished.\n";
	}

	void FVideoDecoder::AudioDecodeLoop()
	{
		if (m_AudioStreamIndex == -1)
			return;

		FfmpegAVFormatContext* formatContext = m_FContext->GetFormatContext();

		while (m_IsRunning) {

			if (m_IsPauseDecode) {

				if (IsRunningDecode() == false)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			
			// 等待 packet
			AVPacket pktLocal;
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Condition.wait(lock, [this] {
					return !m_AudioPacketQueue.empty() || m_IsDemuxFinished;
					});
				if (m_AudioPacketQueue.empty() && m_IsDemuxFinished) {
					m_AudioRingBuffer->WriteFinish();
					break;
				}

				av_packet_ref(&pktLocal, m_AudioPacketQueue.front()->GetPacket());
				m_AudioPacketQueue.pop();
			}

			//std::cout << "m_AudioPacketQueue: " << m_AudioPacketQueue.size() << std::endl;

			// 如果是音频流就解码
			m_AudioCodecContext->SendPacket(&pktLocal);
			av_packet_unref(&pktLocal);
			while (m_AudioCodecContext->ReceiveFrame(*m_AudioFrame) >= 0) {
				//int samples = swr_convert(swr, &m_AudioBuf, m_AudioMaxSamples, (const uint8_t**)m_FfmpegAVFrame->data, m_FfmpegAVFrame->GetNumberOfSamples());
				int samples = swr_convert(
					m_AudioSwrContext->GetPtr(),
					&m_AudioBuf,
					m_AudioMaxSamples,
					m_AudioFrame->GetDataAddress(),
					m_AudioFrame->GetNumberOfSamples()
				);

				if (samples > 0) {
					int channels = 2;
					int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
					int bytes = samples * channels * bps;
					int bufSize = av_samples_get_buffer_size(
						nullptr, channels, samples, AV_SAMPLE_FMT_S16, 1);

					m_AudioRingBuffer->Write(m_AudioBuf, bufSize);

					AVRational tb = formatContext->GetStream(m_AudioStreamIndex)->time_base;
					m_AudioClock = m_AudioFrame->GetBestEffortTimestamp() * av_q2d(tb);
				}

				// 等待音频缓冲区有空间
				while (m_AudioRingBuffer && m_AudioRingBuffer->IsAlmostFull())
				{
					//std::cout << m_IsRunning << std::endl;
					if (m_IsRunning == false)
						return;
					//std::cout << m_IsPauseDecode << std::endl;
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}

	}

	bool FVideoDecoder::VideoDecodeLoop()
	{
		const auto WAIT_TIMEOUT = std::chrono::milliseconds(200);
		uint8_t* frameData = nullptr;
		int lineSize = 0;
		int width = 0;
		int height = 0;
		double pts = 0.f;
		FfmpegAVFormatContext* formatContext = m_FContext->GetFormatContext();

		while (IsRunningDecode()) {
			if (m_IsPauseDecode) {

				if (IsRunningDecode() == false)
					return false;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			AVPacket pktLocal;
			bool gotPacket = false;

			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				// 等待 packet
				{
					if (!m_Condition.wait_for(lock, WAIT_TIMEOUT, [&]() { return !m_VideoPacketQueue.empty() || m_IsDemuxFinished || !IsRunningDecode(); }))
						continue;
					if (!m_VideoPacketQueue.empty()) {
						av_packet_ref(&pktLocal, m_VideoPacketQueue.front()->GetPacket());
						gotPacket = true;
					}
					else if (m_IsDemuxFinished)
					{
						OnFinishedDecode();
						return false;
					}
				}

				// 发送 packet
				if (gotPacket) {
					//int ret = avcodec_send_packet(codecContext, &pktLocal);
					int ret = m_VideoCodecContext->SendPacket(&pktLocal);
					av_packet_unref(&pktLocal);

					if (ret < 0 && ret != AVERROR(EAGAIN)) return false;

					if (ret == AVERROR(EAGAIN)) {
						// 输出队列满：尝试接收并释放一帧，让 decoder 有空间，然后重试（不 pop pkg）
						//AVFrame* tmp = av_frame_alloc();
						//ret = avcodec_receive_frame(codecContext, tmp);
						//if (ret >= 0) {
						//	// Got a frame: convert并返回（注意这并不改变 pkt 在队列里的位置）
						//	sws_scale(swsContext, (const uint8_t* const*)tmp->data,
						//		tmp->linesize, 0, height,
						//		frameRGB->data, frameRGB->linesize);
						//
						//	AVRational tb = formatContext->GetStream(m_VideoStreamIndex)->time_base;
						//	pts = tmp->best_effort_timestamp * av_q2d(tb);
						//	frameData = frameRGB->data[0];
						//	lineSize = frameRGB->linesize[0];
						//	width = this->width;
						//	height = this->height;
						//
						//	av_frame_free(&tmp);
						//	return true;
						//}
						//// 若没有拿到 frame（比如 EAGAIN 再次），释放 tmp 并重试 send（注意 pkt 仍在队列）
						//av_frame_free(&tmp);
						//// 小睡避免 busy-loop
						//std::this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;
					}

					// 弹出队列
					{
						//std::lock_guard<std::mutex> lock(mutex);
						if (!m_VideoPacketQueue.empty()) m_VideoPacketQueue.pop();
						m_Condition.notify_all();
					}
				}
			}

			// 接收 frame
			while (true) {
				if (m_IsPauseDecode) {

					if (IsRunningDecode() == false)
						return false;

					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}

				//std::cout << "m_VideoPacketQueue: " << m_VideoPacketQueue.size() << std::endl;
				int ret = m_VideoCodecContext->ReceiveFrame(*m_VideoFrame);

				if (ret == AVERROR(EAGAIN)) break;
				if (ret == AVERROR_EOF) {
					return false;
				}
				if (ret < 0) 
					return false;

				sws_scale(
					m_VideoSwsContext->GetPtr(), 
					m_VideoFrame->GetDataAddress(), 
					m_VideoFrame->GetPtr()->linesize,
					0, 
					m_VideoFrame->GetPtr()->height,
					m_VideoRGBFrame->GetPtr()->data,
					m_VideoRGBFrame->GetPtr()->linesize
				);
				

				AVRational tb = formatContext->GetStream(m_VideoStreamIndex)->time_base;
				pts = m_VideoFrame->GetBestEffortTimestamp() * av_q2d(tb);
				m_VideoClock = pts;

				frameData = m_VideoRGBFrame->GetPtr()->data[0];
				lineSize = m_VideoRGBFrame->GetPtr()->linesize[0];
				width = m_VideoWidth;
				height = m_VideoHeight;

				// 数据回调
				if (OnVideoDataUpdate)
					OnVideoDataUpdate(frameData, lineSize, width, height, pts);
			}
		}

		return false;
	}

	//void FVideoDecoder::SetLoopDecode(bool enable)
	//{
	//	m_EnableDecodeLoop = enable;
	//}
	//
	//bool FVideoDecoder::IsLoopDecode() const
	//{
	//	return m_EnableDecodeLoop;
	//}

	bool FVideoDecoder::PauseDecode(bool pause)
	{
		m_IsPauseDecode = pause;
		return true;
	}

	bool FVideoDecoder::RestoreDecode()
	{
		m_IsPauseDecode = false;

		if (IsRunningDecode() == false || m_IsDemuxFinished == true || m_IsFinishedDecode == true)
		{
			return StartDecode();
		}

		return true;
	}

	bool FVideoDecoder::RestartDecode()
	{
		if (Seek(0) == false)
			return false;

		if (RestoreDecode() == false)
			return false;
	}

	bool FVideoDecoder::IsPauseDecode() const
	{
		return m_IsPauseDecode;
	}

	bool FVideoDecoder::IsFinishedDecode() const
	{
		return m_IsFinishedDecode;
	}

	double FVideoDecoder::GetPlaybackTime() const
	{
		// 优先使用音频时钟
		if (HasAudioStream())
		{
			return m_AudioClock.load();
		}

		// 如果没有音频，则使用视频时钟
		if (m_VideoStreamIndex != -1)
		{
			//std::cout << "m_VideoClock: " << m_VideoClock.load() << std::endl;
			return m_VideoClock.load();
		}

		return 0.0;
	}
}
