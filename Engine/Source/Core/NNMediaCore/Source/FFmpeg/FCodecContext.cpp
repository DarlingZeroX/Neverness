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

#include "FFmpeg/FCodecContext.h"
extern "C" {
#include <libavutil/pixdesc.h>
}

namespace NN::Core {

	FfmpegAVCodecContext::FfmpegAVCodecContext(FfmpegAVFormatContext& context, uint index, bool isVideo)
	{
		// 音频
		const auto* decoder = avcodec_find_decoder(context.GetStream(index)->codecpar->codec_id);
		m_AVCodecContext = avcodec_alloc_context3(decoder);
		avcodec_parameters_to_context(m_AVCodecContext, context.GetStream(index)->codecpar);
		avcodec_open2(m_AVCodecContext, nullptr, nullptr);


		auto* codecParameters = context.GetStream(index)->codecpar;
		// 查找视频解码器
		const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
		if (!codec) {
			std::cerr << "Failed to find codec" << std::endl;
			return;
		}

		// 分配解码器上下文
		m_AVCodecContext = avcodec_alloc_context3(codec);
		if (!m_AVCodecContext) {
			std::cerr << "Failed to allocate codec context" << std::endl;
			return;
		}

		// 填充解码器上下文
		if (avcodec_parameters_to_context(m_AVCodecContext, codecParameters) < 0) {
			std::cerr << "Failed to copy codec parameters to codec context" << std::endl;
			return;
		}

		// 如果像素格式未指定，尝试强制设置常见格式
		if (isVideo)
		{
			if (codecParameters->format == AV_PIX_FMT_NONE) {
				std::cout << "Pixel format not specified in stream, trying default formats" << std::endl;

				// 对于H.264视频，常见格式是YUV420P
				if (codecParameters->codec_id == AV_CODEC_ID_H264) {
					m_AVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
					std::cout << "Forcing H.264 pixel format to AV_PIX_FMT_YUV420P" << std::endl;
				}
			}
		}

		// 打开解码器
		if (avcodec_open2(m_AVCodecContext, codec, nullptr) < 0) {
			std::cerr << "Failed to open codec" << std::endl;
			return;
		}
	}

	FfmpegAVCodecContext::~FfmpegAVCodecContext()
	{
		// 在类的析构函数中
		if (m_AVCodecContext) {
			avcodec_free_context(&m_AVCodecContext);
			m_AVCodecContext = nullptr;
		}
	}

	Ref<FfmpegAVCodecContext> FfmpegAVCodecContext::Create(FfmpegAVFormatContext& context, uint index)
	{
		return MakeRef<FfmpegAVCodecContext>(context, index);
	}

	AVChannelLayout FfmpegAVCodecContext::GetCHLayout() const
	{
		return m_AVCodecContext->ch_layout;
	}

	AVSampleFormat FfmpegAVCodecContext::GetSampleFormat() const
	{
		return m_AVCodecContext->sample_fmt;
	}

	int FfmpegAVCodecContext::GetSampleRate() const
	{
		return m_AVCodecContext->sample_rate;
	}

	void FfmpegAVCodecContext::FlushBuffers() const
	{
		return avcodec_flush_buffers(m_AVCodecContext);
	}

	int FfmpegAVCodecContext::SendPacket(FfmpegAVPacket& packet) const
	{
		return avcodec_send_packet(m_AVCodecContext, packet.GetPacket());
	}

	int FfmpegAVCodecContext::SendPacket(AVPacket* packet) const
	{
		return avcodec_send_packet(m_AVCodecContext, packet);
	}

	int FfmpegAVCodecContext::ReceiveFrame(FfmpegAVFrame& frame) const
	{
		return avcodec_receive_frame(m_AVCodecContext, frame.GetPtr());
	}

	bool FfmpegAVCodecContext::SetPixelFormat(AVCodecParameters* param)
	{
		if (m_AVCodecContext == nullptr)
			return false;

		// 再次检查像素格式
		if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_NONE) {
			std::cerr << "Decoder still didn't set a pixel format" << std::endl;

			// 尝试从编解码器参数获取
			if (param->format != AV_PIX_FMT_NONE) {
				m_AVCodecContext->pix_fmt = static_cast<AVPixelFormat>(param->format);
				std::cout << "Using pixel format from codec parameters: "
					<< av_get_pix_fmt_name(m_AVCodecContext->pix_fmt) << std::endl;
			}
			else {
				// 最后尝试设置一个安全的默认值
				m_AVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
				std::cout << "Forcing default pixel format to AV_PIX_FMT_YUV420P" << std::endl;
			}
		}
	}

	AVPixelFormat FfmpegAVCodecContext::GetPixelFormat() const
	{
		return m_AVCodecContext->pix_fmt;
	}

	int FfmpegAVCodecContext::GetVideoWidth() const
	{
		return m_AVCodecContext->width;
	}

	int FfmpegAVCodecContext::GetVideoHeight() const
	{
		return m_AVCodecContext->height;
	}

	int FfmpegAVCodecContext::Open2() const
	{
		return avcodec_open2(m_AVCodecContext, nullptr, nullptr);
	}

}