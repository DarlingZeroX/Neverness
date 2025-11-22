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

#include "Resource/Audio/AudioRingBuffer.h"

namespace VisionGal {

	AudioRingBuffer::AudioRingBuffer(size_t cap)
		: buffer(cap), capacity(cap) {
	}

	size_t AudioRingBuffer::Write(const uint8_t* data, size_t len) {
		std::lock_guard<std::mutex> lock(mutex);
		size_t written = 0;
		while (written < len && size < capacity) {
			buffer[tail] = data[written++];
			tail = (tail + 1) % capacity;
			size++;
		}

		if (IsAlmostFull()) {
			//std::cerr << "[Audio] RingBuffer almost full (" << size << "/" << buffer.size() << ")" << std::endl;
		}

		return written;
	}

	size_t AudioRingBuffer::Read(uint8_t* out, size_t len) {
		std::lock_guard<std::mutex> lock(mutex);
		size_t read = 0;
		while (read < len && size > 0) {
			out[read++] = buffer[head];
			head = (head + 1) % capacity;
			size--;
		}


		if (IsAlmostFull() == false) {
			//std::cout << "[Audio] RingBuffer size (" << size << "/" << buffer.size() << ")" << std::endl;
		}

		return read;
	}

	size_t AudioRingBuffer::Available() const {
		std::lock_guard<std::mutex> lock(mutex);
		return size;
	}

	bool AudioRingBuffer::IsAlmostFull() const
	{
		return size >= capacity - 128 * 1024;
	}

	void AudioRingBuffer::Stop()
	{
		m_IsFinish = true;
	}

	bool AudioRingBuffer::IsFinish() const
	{
		return m_IsFinish;
	}

	void AudioRingBuffer::WriteFinish()
	{
		m_IsWriteFinish = true;
	}

	bool AudioRingBuffer::IsWriteFinish() const
	{
		return m_IsWriteFinish;
	}

	void AudioRingBuffer::Reset()
	{
		std::lock_guard<std::mutex> lock(mutex);
		size = 0;
		head = 0;
		tail = 0;
		m_IsFinish = false;
		m_IsWriteFinish = false;
	}
}
