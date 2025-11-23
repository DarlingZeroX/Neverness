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

#pragma once
#include "../../Core/Core.h"

extern "C" {
#include <libswscale/swscale.h>
}

namespace VisionGal {

	struct FfmpegSwsContext
	{
		FfmpegSwsContext() = default;
		~FfmpegSwsContext();

		static Ref<FfmpegSwsContext> Create(int srcW, int srcH, enum AVPixelFormat srcFormat,
			int dstW, int dstH, enum AVPixelFormat dstFormat,
			int flags, SwsFilter* srcFilter,
			SwsFilter* dstFilter, const double* param
		);

		SwsContext* GetPtr() const { return m_SwsContext; }
	private:
		bool Initialize(int srcW, int srcH, enum AVPixelFormat srcFormat,
			int dstW, int dstH, enum AVPixelFormat dstFormat,
			int flags, SwsFilter* srcFilter,
			SwsFilter* dstFilter, const double* param);

		SwsContext* m_SwsContext = nullptr;
	};

}
