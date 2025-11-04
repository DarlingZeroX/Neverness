#pragma once
#include <vector>
#include <mutex>

namespace VisionGal {

	// 简单线程安全环形缓冲
	class AudioRingBuffer {
	public:
		AudioRingBuffer(size_t capacity);

		size_t Write(const uint8_t* data, size_t len);
		size_t Read(uint8_t* out, size_t len);
		size_t Available() const;
		bool IsAlmostFull() const;
		void Stop();
		bool IsFinish() const;
		void WriteFinish();
		bool IsWriteFinish() const;
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
