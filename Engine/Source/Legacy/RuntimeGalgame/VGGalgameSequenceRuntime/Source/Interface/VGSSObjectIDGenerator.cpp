/*
 * VGSSObjectIDGenerator 实现
 */

#include "VGSSObjectIDGenerator.h"

namespace VisionGal
{
	uint32_t VGSSObjectIDGenerator::GenerateUniqueRawId() noexcept
	{
		// fetch_add 返回递增前的值；m_NextRawId 初值为 1 → 首次返回 1，随后 2、3…
		return m_NextRawId.fetch_add(1u, std::memory_order_acq_rel);
	}

	VGSSCharacterObjectID VGSSObjectIDGenerator::GenerateCharacterID() noexcept
	{
		return GenerateUniqueRawId();
	}

	VGSSSpriteObjectID VGSSObjectIDGenerator::GenerateSpriteID() noexcept
	{
		return GenerateUniqueRawId();
	}

	VGSSAudioObjectID VGSSObjectIDGenerator::GenerateAudioID() noexcept
	{
		return GenerateUniqueRawId();
	}

	VGSSVideoObjectID VGSSObjectIDGenerator::GenerateVideoID() noexcept
	{
		return GenerateUniqueRawId();
	}
}
