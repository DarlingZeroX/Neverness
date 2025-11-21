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

#include "Resource/FFmpeg/FIOContext.h"

namespace VisionGal {

	FfmpegAVIOContext::FfmpegAVIOContext(void* opaque, ReadCallback read_cb, SeekCallback seek_cb, int bufferSize)
		: m_Opaque(opaque), m_BufferSize(bufferSize)
	{
		if (!read_cb) {
			throw std::invalid_argument("Read callback must not be null");
		}

		// 分配缓冲区
		m_Buffer = static_cast<uint8_t*>(av_malloc(m_BufferSize));
		if (!m_Buffer) {
			throw std::bad_alloc();
		}

		// 创建 AVIOContext
		m_AVIOContext = avio_alloc_context(
			m_Buffer,
			m_BufferSize,
			0, // write_flag = 0
			m_Opaque,
			read_cb,
			nullptr, // 写回调
			seek_cb
		);

		if (!m_AVIOContext) {
			av_free(m_Buffer);
			m_Buffer = nullptr;
			throw std::runtime_error("Failed to allocate AVIOContext");
		}
	}

	FfmpegAVIOContext::FfmpegAVIOContext(void* opaque, ReadCallback read_cb, WriteCallback write_cb,
		SeekCallback seek_cb, int bufferSize)
		: m_Opaque(opaque), m_BufferSize(bufferSize)
	{
		if (!read_cb && !write_cb) {
			throw std::invalid_argument("At least one of read or write callback must not be null");
		}

		m_Buffer = static_cast<uint8_t*>(av_malloc(m_BufferSize));
		if (!m_Buffer) {
			throw std::bad_alloc();
		}

		m_AVIOContext = avio_alloc_context(
			m_Buffer,
			m_BufferSize,
			write_cb != nullptr ? 1 : 0,
			m_Opaque,
			read_cb,
			write_cb,
			seek_cb
		);

		if (!m_AVIOContext) {
			av_free(m_Buffer);
			m_Buffer = nullptr;
			throw std::runtime_error("Failed to allocate AVIOContext");
		}
	}

	FfmpegAVIOContext::~FfmpegAVIOContext()
	{
		if (m_AVIOContext) {
			// 释放 AVIOContext 内部 buffer
			av_freep(&m_AVIOContext->buffer);
			avio_context_free(&m_AVIOContext);
			m_AVIOContext = nullptr;
			m_Buffer = nullptr;
		}
	}

	FfmpegAVIOContext::FfmpegAVIOContext(FfmpegAVIOContext&& other) noexcept
		: m_Opaque(other.m_Opaque), m_AVIOContext(other.m_AVIOContext), m_Buffer(other.m_Buffer), m_BufferSize(other.m_BufferSize)
	{
		other.m_AVIOContext = nullptr;
		other.m_Buffer = nullptr;
	}

	FfmpegAVIOContext& FfmpegAVIOContext::operator=(FfmpegAVIOContext&& other) noexcept
	{
		if (this != &other) {
			this->~FfmpegAVIOContext();

			m_Opaque = other.m_Opaque;
			m_AVIOContext = other.m_AVIOContext;
			m_Buffer = other.m_Buffer;
			m_BufferSize = other.m_BufferSize;

			other.m_AVIOContext = nullptr;
			other.m_Buffer = nullptr;
		}
		return *this;
	}

}
