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

#include "FFmpeg/FSwsContext.h"
#include <iostream>

namespace Horizon {
	FfmpegSwsContext::~FfmpegSwsContext()
	{
		if (m_SwsContext) {
			sws_freeContext(m_SwsContext);
			m_SwsContext = nullptr;
		}
	}

	bool FfmpegSwsContext::Initialize(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH,
		enum AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter, const double* param)
	{
		m_SwsContext = sws_getContext(srcW, srcH, srcFormat,
			dstW, dstH, dstFormat,
			flags, srcFilter, dstFilter, param);

		if (!m_SwsContext) {
			std::cerr << "Failed to initialize sws context" << std::endl;
			return false;
		}

		return true;
	}

	Ref<FfmpegSwsContext> FfmpegSwsContext::Create(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH,
	                                               enum AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter, const double* param)
	{
		auto context = CreateRef<FfmpegSwsContext>();
		if (context->Initialize(srcW, srcH, srcFormat, dstW, dstH, dstFormat, flags, srcFilter, dstFilter, param) == false)
			return nullptr;

		return context;
	}

}
