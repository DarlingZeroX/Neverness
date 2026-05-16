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

#include "FFmpeg/FFrame.h"
#include <iostream>
extern "C" {
#include <libavutil/imgutils.h>
}

namespace NN::Core {

	FfmpegAVFrame::FfmpegAVFrame()
	{
		m_AVFrame = av_frame_alloc();
		if (m_AVFrame == nullptr) {
			std::cerr << "Failed to allocate frame" << std::endl;
		}
	}

	FfmpegAVFrame::~FfmpegAVFrame()
	{
		if (m_AVFrame) {
			av_frame_free(&m_AVFrame);
			m_AVFrame = nullptr;
		}
	}

	int FfmpegAVFrame::VideoImageFillArrays(const FfmpegBuffer& buffer, AVPixelFormat pix_fmt, int width, int height, int align)
	{
		return av_image_fill_arrays(
			m_AVFrame->data, 
			m_AVFrame->linesize, 
			buffer.GetBufferPtr(),
			pix_fmt, width, height, align
		);
	}
}
