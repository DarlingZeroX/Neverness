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

#include "FFmpeg/FChannelLayout.h"

namespace NN::Core {
	FfmpegAVChannelLayout::FfmpegAVChannelLayout(int channels)
	{
		av_channel_layout_default(&m_Layout, 2);
	}

	FfmpegAVChannelLayout::FfmpegAVChannelLayout(const AVChannelLayout& layout)
	{
		m_Layout = layout;
	}

	AVChannelLayout& FfmpegAVChannelLayout::GetRef()
	{
		return m_Layout;
	}
}
