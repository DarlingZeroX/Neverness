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
#include <mutex>
#include "../Interface/IAudioDataBuffer.h"

namespace VisionGal {

	// 简单线程安全环形缓冲
	class AudioRingBuffer: public IAudioDataBuffer{
	public:
		AudioRingBuffer(size_t capacity);

		size_t Write(const uint8_t* data, size_t len) override;
		size_t Read(uint8_t* out, size_t len) override;
		size_t Available() const override;
		bool IsAlmostFull() const override;
		void Stop() override;
		bool IsFinish() const override;
		void WriteFinish() override;
		bool IsWriteFinish() const override;
	private:
		std::vector<uint8_t> buffer;
		size_t head = 0;
		size_t tail = 0;
		size_t size = 0;
		size_t capacity = 0;
		mutable std::mutex mutex;
		bool m_IsFinish = false;
		bool m_IsWriteFinish = false;
	};
}
