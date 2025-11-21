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
#include "FChannelLayout.h"
#include "FCodecContext.h"

extern "C" {
#include <libswresample/swresample.h>
}

namespace VisionGal {

	struct FfmpegSwrContext
	{
		FfmpegSwrContext() = default;
		~FfmpegSwrContext() ;

		static Ref<FfmpegSwrContext> Create(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout, FfmpegAVCodecContext& ctx);

		SwrContext* GetPtr() const { return m_SwrContext; }
	private:
		bool Initialize(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout, FfmpegAVCodecContext& ctx);

		SwrContext* m_SwrContext = nullptr;
	};

}
