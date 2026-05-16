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
#include "FFmpeg/FBuffer.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace NN::Core {
	FfmpegBuffer::FfmpegBuffer(size_t size)
		:m_Size(size)
	{
		// av_malloc 默认返回 32 字节对齐的内存
		m_Buffer = static_cast<uint8_t*>(av_malloc(size));
		if (!m_Buffer) throw std::bad_alloc();
	}

	FfmpegBuffer::~FfmpegBuffer()
	{
		if (m_Buffer) {
			av_free(m_Buffer);
			m_Buffer = nullptr;
		}
	}

	Ref<FfmpegBuffer> FfmpegBuffer::New(size_t size)
	{
		return MakeRef<FfmpegBuffer>(size);
	}

	size_t FfmpegBuffer::Size() const
	{
		return m_Size;
	}

	unsigned char* FfmpegBuffer::GetBufferPtr() const
	{
		return m_Buffer;
	}
}


