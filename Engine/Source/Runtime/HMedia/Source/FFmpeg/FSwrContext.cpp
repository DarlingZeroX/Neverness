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

#include "FFmpeg/FSwrContext.h"

namespace Horizon {
	FfmpegSwrContext::~FfmpegSwrContext()
	{
		if (m_SwrContext) {
			swr_free(&m_SwrContext);
			m_SwrContext = nullptr;
		}
	}

	Ref<FfmpegSwrContext> FfmpegSwrContext::Create(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout,
	                                               FfmpegAVCodecContext& ctx)
	{
		auto context = MakeRef<FfmpegSwrContext>();
		if (context->Initialize(inLayout, outLayout, ctx) == false)
			return nullptr;

		return context;
	}

	bool FfmpegSwrContext::Initialize(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout,
	                                  FfmpegAVCodecContext& context)
	{
		if (swr_alloc_set_opts2(
			&m_SwrContext,
			&outLayout.GetRef(), AV_SAMPLE_FMT_S16, 44100,
			&inLayout.GetRef(), context.GetSampleFormat(), context.GetSampleRate(),
			0, nullptr) < 0)
		{
			std::cerr << "Failed to alloc swr" << std::endl;
			return false;
		}

		if (swr_init(m_SwrContext) < 0) {
			std::cerr << "Failed to init swr" << std::endl;
			return false;
		}

		return true;
	}
}
