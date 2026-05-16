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
//#include "../../Core/Core.h"
#include "FFormatContext.h"
#include "FPacket.h"
#include "FFrame.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace Horizon {
	struct FfmpegAVCodecContext
	{
		FfmpegAVCodecContext(FfmpegAVFormatContext& context, uint index, bool isVideo = false);
		~FfmpegAVCodecContext();

		static Ref<FfmpegAVCodecContext> Create(FfmpegAVFormatContext& context, uint index);

		AVChannelLayout GetCHLayout() const;
		AVSampleFormat GetSampleFormat() const;
		int GetSampleRate() const;
		void FlushBuffers() const;

		int SendPacket(FfmpegAVPacket& packet) const;
		int SendPacket(AVPacket* packet) const;
		int ReceiveFrame(FfmpegAVFrame& frame) const;

		bool SetPixelFormat(AVCodecParameters* param);
		AVPixelFormat GetPixelFormat() const;

		int GetVideoWidth() const;
		int GetVideoHeight() const;
	private:
		int Open2() const;

		AVCodecContext* m_AVCodecContext = nullptr;
	};
}
