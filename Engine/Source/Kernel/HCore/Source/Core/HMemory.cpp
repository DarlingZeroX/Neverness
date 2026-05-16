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


#include "HMemory.h"

#include <cassert>
#include <typeinfo>
#include <memory>

namespace Horizon
{

	HMemoryBuffer::HMemoryBuffer() noexcept
		:
		m_buffer(nullptr),
		m_bufferSize(0),
		m_hasOwnership(false)
	{ }

	HMemoryBuffer::HMemoryBuffer(int64 bufferSize) noexcept
		:
		m_buffer(new uint8[bufferSize]),
		m_bufferSize(bufferSize),
		m_hasOwnership(true)
	{
		//mi_option_set(mi_option_t::mi_option_show_errors, 1);
		//m_buffer =
	}

	HMemoryBuffer::HMemoryBuffer(byte* buffer, int64 bufferSize, InitType initType) noexcept
		:
		m_buffer((initType != InitType::Copy) ? (buffer) : (new byte[bufferSize])),
		m_bufferSize(bufferSize),
		m_hasOwnership(initType != InitType::View)
	{
		if (initType == InitType::Copy)
			memcpy(m_buffer, buffer, bufferSize);
		else
		{
			// if taking ownership or just viewing, must have been allocated with new uint8[]
			assert(typeid(m_buffer) == typeid(buffer));
		}
	}

	HMemoryBuffer::HMemoryBuffer(const HMemoryBuffer& copySrc) noexcept
		:
		m_buffer(new uint8[copySrc.GetSize()]),
		m_bufferSize(copySrc.GetSize()),
		m_hasOwnership(true)
	{
	}

	HMemoryBuffer::HMemoryBuffer(HMemoryBuffer&& moveSrc) noexcept
		:
		m_buffer(moveSrc.GetData()),
		m_bufferSize(moveSrc.GetSize()),
		m_hasOwnership(moveSrc.m_hasOwnership)
	{
		moveSrc.m_buffer = nullptr;
		moveSrc.m_bufferSize = 0;
		moveSrc.m_hasOwnership = false;
	}

	HMemoryBuffer::~HMemoryBuffer() noexcept
	{
		Clear();
	}

	HMemoryBuffer& HMemoryBuffer::operator=(const HMemoryBuffer& copySrc) noexcept
	{
		// avoid copy if buffers same size!
		if (copySrc.m_bufferSize != m_bufferSize)
		{
			Clear();
			m_buffer = new uint8[copySrc.GetSize()];
			m_bufferSize = copySrc.GetSize();
		}
		m_hasOwnership = true;
		memcpy(m_buffer, copySrc.GetData(), m_bufferSize);

		return *this;
	}

	bool HMemoryBuffer::operator==(const HMemoryBuffer& other) const noexcept
	{
		if (m_bufferSize != other.m_bufferSize)
			return false;

		// comparison logic goes here
		return memcmp(m_buffer, other.m_buffer, m_bufferSize) == 0;
	}

	void HMemoryBuffer::Clear()
	{
		if (m_buffer != nullptr && m_hasOwnership)
			delete[] m_buffer;
		m_buffer = nullptr;
		m_bufferSize = 0;
		m_hasOwnership = false;
	}
}