/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <HCore/Interface/HCoreTypes.h>

namespace VisionGal::Editor
{
	enum class SequenceDirtyRegionFlags : uint32_t
	{
		None = 0,
		Structure = 1u << 0,
		Property = 1u << 1,
		Validation = 1u << 2,
		RuntimeOverlay = 1u << 3,
		SearchIndex = 1u << 4,
		TimelineLayout = 1u << 5,
		OutlinerLayout = 1u << 6,
	};

	inline SequenceDirtyRegionFlags operator|(SequenceDirtyRegionFlags a, SequenceDirtyRegionFlags b)
	{
		return static_cast<SequenceDirtyRegionFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline SequenceDirtyRegionFlags& operator|=(SequenceDirtyRegionFlags& a, SequenceDirtyRegionFlags b)
	{
		a = a | b;
		return a;
	}

	inline SequenceDirtyRegionFlags operator&(SequenceDirtyRegionFlags a, SequenceDirtyRegionFlags b)
	{
		return static_cast<SequenceDirtyRegionFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	inline SequenceDirtyRegionFlags& operator&=(SequenceDirtyRegionFlags& a, SequenceDirtyRegionFlags b)
	{
		a = a & b;
		return a;
	}

	//BITFLAG_ENUM_CLASS_HELPER(SequenceDirtyRegionFlags)
}
