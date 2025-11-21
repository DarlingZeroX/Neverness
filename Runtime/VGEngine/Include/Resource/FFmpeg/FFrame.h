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
#include "FContext.h"

extern "C" {
#include <libswresample/swresample.h>
}

namespace VisionGal {

	struct FfmpegAVFrame
	{
		FfmpegAVFrame();
		~FfmpegAVFrame();

		//static Ref<FfmpegSwrContext> Create(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout, FfmpegAVCodecContext& ctx);

		AVFrame* GetPtr() const { return m_AVFrame; };

		AVFrame** GetAddress() { return &m_AVFrame; };

		uint8_t** GetDataAddress() const { return reinterpret_cast<uint8_t**>(&m_AVFrame->data); }

		int64_t GetBestEffortTimestamp() const { return  m_AVFrame->best_effort_timestamp; }

		int GetNumberOfSamples() const { return m_AVFrame->nb_samples; }
	private:
		//bool Initialize(FfmpegAVChannelLayout& inLayout, FfmpegAVChannelLayout& outLayout, FfmpegAVCodecContext& ctx);

		AVFrame* m_AVFrame = nullptr;
	};

}
