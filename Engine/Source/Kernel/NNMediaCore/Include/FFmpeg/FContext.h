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
#include "FBuffer.h"
#include "FDictionary.h"
#include "FIOContext.h"
#include "FFormatContext.h"
#include "../Common/FFmpegIOContext.h"

namespace Horizon {
	struct FfmpegContext
	{
		FfmpegContext() = default;
		~FfmpegContext() = default;

		static Ref<FfmpegContext> Create(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath);

		int FindAudioStreamIndex() const;
		int FindVideoStreamIndex() const;

		FfmpegAVFormatContext* GetFormatContext() const;
	private:
		bool Open(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath);

		VFSFFmpegIOContext m_VFSIoContext;

		Ref<FfmpegAVFormatContext> m_AVFormatContext;
		//Ref<FfmpegBuffer> m_IOBuffer;
		Ref<FfmpegAVIOContext> m_AVIOContext;
	};
}
