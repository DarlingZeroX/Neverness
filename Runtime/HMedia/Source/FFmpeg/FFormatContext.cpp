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

#include "FFmpeg/FFormatContext.h"

namespace Horizon {

	FfmpegAVFormatContext::FfmpegAVFormatContext()
	{
		m_FormatContext = avformat_alloc_context();
	}

	FfmpegAVFormatContext::~FfmpegAVFormatContext()
	{
		if (m_FormatContext) {
			avformat_close_input(&m_FormatContext);
			m_FormatContext = nullptr;
		}
	}

	Ref<FfmpegAVFormatContext> FfmpegAVFormatContext::Create()
	{
		return CreateRef<FfmpegAVFormatContext>();
	}

	void FfmpegAVFormatContext::SetIOContext(FfmpegAVIOContext& ioCtx) const
	{
		m_FormatContext->pb = ioCtx.Get();
	}

	AVStream* FfmpegAVFormatContext::GetStream(uint index) const
	{
		return m_FormatContext->streams[index];
	}

	int FfmpegAVFormatContext::OpenInput(FfmpegAVDictionary& options)
	{
		return avformat_open_input(&m_FormatContext, nullptr, nullptr, options.GetAddress());
	}

	int FfmpegAVFormatContext::ReadFrame(FfmpegAVPacket& packet) const
	{
		return av_read_frame(m_FormatContext, packet.GetPacket());
	}

	int FfmpegAVFormatContext::FindStreamInfo() const
	{
		return avformat_find_stream_info(m_FormatContext, nullptr);
	}

	int FfmpegAVFormatContext::SeekFrame(int stream_index, int64_t timestamp, int flags) const
	{
		return av_seek_frame(m_FormatContext, stream_index, timestamp, flags);
	}

	int FfmpegAVFormatContext::FindAudioStreamIndex() const
	{
		int streamIndex = -1;

		for (unsigned int i = 0; i < m_FormatContext->nb_streams; i++) {
			if (m_FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				streamIndex = i;
			}
		}

		return streamIndex;
	}

	int FfmpegAVFormatContext::FindVideoStreamIndex() const
	{
		int streamIndex = -1;

		for (unsigned int i = 0; i < m_FormatContext->nb_streams; i++) {
			if (m_FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				streamIndex = i;
			}
		}

		return streamIndex;
	}

}