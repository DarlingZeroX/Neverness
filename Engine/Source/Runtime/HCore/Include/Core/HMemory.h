// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../CoreModuleDefinitions.h"
#include "../Core/HCoreTypes.h"

namespace Horizon
{
	// Just a simple generic self-contained memory buffer helper class, for passing data as argument, etc.
	class CORE_MODULE_API HMemoryBuffer
	{
		byte* m_buffer;
		uint64                   m_bufferSize;
		bool                    m_hasOwnership;     // if it has ownership, will delete[] and expect it to be of uint8[] type - could expand with custom deallocator if needed

	public:
		enum class InitType
		{
			Copy,               // copy buffer (caller is free to release/modify the memory pointer after this call)
			TakeOwnership,      // take buffer pointer and delete[] when object is destroyed (caller should not ever use the memory after this call)
			View,               // take buffer pointer but don't delete[] when object is destroyed (caller should not ever use the memory after this call)
		};

	public:
		HMemoryBuffer() noexcept;
		HMemoryBuffer(int64 bufferSize) noexcept;
		HMemoryBuffer(byte* buffer, int64 bufferSize, InitType initType = InitType::Copy) noexcept;
		HMemoryBuffer(const HMemoryBuffer& copySrc) noexcept;
		HMemoryBuffer(HMemoryBuffer&& moveSrc) noexcept;
		~HMemoryBuffer() noexcept;
	public:
		HMemoryBuffer& operator = (const HMemoryBuffer& copySrc) noexcept;
		bool operator == (const HMemoryBuffer& other) const noexcept;
		void Clear();
	public:
		byte* GetData() const noexcept { return m_buffer; }
		int64       GetSize() const noexcept { return m_bufferSize; }
	};
}