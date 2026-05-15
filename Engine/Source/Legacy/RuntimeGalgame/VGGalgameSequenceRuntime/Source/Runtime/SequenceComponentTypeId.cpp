/*
 * SequenceComponentTypeID — FNV-1a 64 实现
 */

#include "Runtime/SequenceComponentTypeId.h"

namespace VisionGal
{
	namespace
	{
		constexpr SequenceComponentTypeID kFnvOffsetBasis = 14695981039346656037ULL;
		constexpr SequenceComponentTypeID kFnvPrime = 1099511628211ULL;
	}

	SequenceComponentTypeID MakeSequenceComponentTypeIDFromTypeName(const std::string_view typeName)
	{
		SequenceComponentTypeID h = kFnvOffsetBasis;
		for (const unsigned char c : typeName)
		{
			h ^= static_cast<SequenceComponentTypeID>(c);
			h *= kFnvPrime;
		}
		return h;
	}
}
