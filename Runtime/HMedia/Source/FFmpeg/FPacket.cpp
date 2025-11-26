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

#include "FFmpeg/FPacket.h"

namespace Horizon {
	FfmpegAVPacket::FfmpegAVPacket()
	{
		m_Packet = av_packet_alloc();
	}

	FfmpegAVPacket::~FfmpegAVPacket()
	{
		if (m_Packet)
		{
			av_packet_free(&m_Packet);
			m_Packet = nullptr;
		}
	}

	int FfmpegAVPacket::GetStreamIndex() const
	{
		return m_Packet->stream_index;
	}
}
