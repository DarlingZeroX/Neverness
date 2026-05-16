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
#include "FBuffer.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace NN::Core {

	class FfmpegAVIOContext {
	public:
		using ReadCallback = int(*)(void* opaque, uint8_t* buf, int buf_size);
		using WriteCallback = int(*)(void* opaque, const uint8_t* buf, int buf_size);
		using SeekCallback = int64_t(*)(void* opaque, int64_t offset, int whence);

		// 构造只读 AVIOContext
		FfmpegAVIOContext(void* opaque, ReadCallback read_cb, SeekCallback seek_cb = nullptr, int bufferSize = 4096);
		FfmpegAVIOContext(void* opaque, ReadCallback read_cb, WriteCallback write_cb, SeekCallback seek_cb = nullptr, int bufferSize = 4096);			// 构造可读写 AVIOContext
		~FfmpegAVIOContext();
		FfmpegAVIOContext(const FfmpegAVIOContext&) = delete;			// 禁止拷贝
		FfmpegAVIOContext& operator=(const FfmpegAVIOContext&) = delete;
		FfmpegAVIOContext(FfmpegAVIOContext&& other) noexcept;			// 允许移动

		FfmpegAVIOContext& operator=(FfmpegAVIOContext&& other) noexcept;

		AVIOContext* Get() const { return m_AVIOContext; }
	private:
		void* m_Opaque = nullptr;
		AVIOContext* m_AVIOContext = nullptr;
		uint8_t* m_Buffer = nullptr;
		int m_BufferSize = 0;
	};

}
