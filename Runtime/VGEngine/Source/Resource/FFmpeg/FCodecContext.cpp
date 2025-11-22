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

#include "Resource/FFmpeg/FCodecContext.h"

namespace VisionGal {

	FfmpegAVCodecContext::FfmpegAVCodecContext(FfmpegAVFormatContext& context, uint index)
	{
		// 音频
		const auto* decoder = avcodec_find_decoder(context.GetStream(index)->codecpar->codec_id);
		m_AVCodecContext = avcodec_alloc_context3(decoder);
		avcodec_parameters_to_context(m_AVCodecContext, context.GetStream(index)->codecpar);
		avcodec_open2(m_AVCodecContext, nullptr, nullptr);
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
		return CreateRef<FfmpegAVCodecContext>(context, index);
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

	int FfmpegAVCodecContext::ReceiveFrame(FfmpegAVFrame& frame) const
	{
		return avcodec_receive_frame(m_AVCodecContext, frame.GetPtr());
	}

	int FfmpegAVCodecContext::Open2() const
	{
		return avcodec_open2(m_AVCodecContext, nullptr, nullptr);
	}

}