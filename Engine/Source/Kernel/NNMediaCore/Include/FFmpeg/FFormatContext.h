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
#include <NNKernel/Interface/HCore.h>
#include "FDictionary.h"
#include "FIOContext.h"
#include "FPacket.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace Horizon {

	struct FfmpegAVFormatContext
	{
		FfmpegAVFormatContext();
		~FfmpegAVFormatContext();

		static Ref<FfmpegAVFormatContext> Create();

		void SetIOContext(FfmpegAVIOContext& ioCtx) const;

		// 打开输入文件
		int OpenInput(FfmpegAVDictionary& options);

		int ReadFrame(FfmpegAVPacket& packet) const;

		// 读取文件信息,获取流信息
		int FindStreamInfo() const;

		int SeekFrame(int stream_index, int64_t timestamp, int flags) const;

		int FindAudioStreamIndex() const;
		int FindVideoStreamIndex() const;

		AVStream* GetStream(uint index) const;

		double GetDuration() const { return m_FormatContext->duration; }

		// 需要释放的指针
		AVFormatContext* m_FormatContext = nullptr;
	};
}
