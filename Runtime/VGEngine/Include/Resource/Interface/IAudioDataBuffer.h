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
#include <vector>

namespace VisionGal {

	// 音频解码器接口
	struct IAudioDataBuffer {
		virtual ~IAudioDataBuffer() = default;

		virtual size_t Write(const uint8_t* data, size_t len) = 0;
		virtual size_t Read(uint8_t* out, size_t len) = 0;
		virtual size_t Available() const = 0;
		virtual bool IsAlmostFull() const = 0;
		virtual void Stop() = 0;
		virtual bool IsFinish() const = 0;
		virtual void WriteFinish() = 0;
		virtual bool IsWriteFinish() const = 0;
	};
}
