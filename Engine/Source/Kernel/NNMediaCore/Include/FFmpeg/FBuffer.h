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
#include "../HMediaConfig.h"
#include <NNKernel/Interface/HConfig.h>

namespace Horizon {

	struct FfmpegBuffer
	{
		FfmpegBuffer(size_t size);
		~FfmpegBuffer();
		FfmpegBuffer(const FfmpegBuffer&) = delete;
		FfmpegBuffer& operator=(const FfmpegBuffer&) = delete;
		FfmpegBuffer(FfmpegBuffer&&) = delete;
		FfmpegBuffer& operator=(FfmpegBuffer&&) = delete;

		static Ref<FfmpegBuffer> New(size_t size);

		size_t Size() const;
		unsigned char* GetBufferPtr() const;
	private:
		unsigned char* m_Buffer = nullptr;
		size_t m_Size;
	};
}
