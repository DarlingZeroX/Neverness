/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Schema/SequencePropertyValue.h"

#include <variant>

namespace VisionGal::Editor
{
	bool SequencePropertyValuesEqual(const SequencePropertyValue& a, const SequencePropertyValue& b)
	{
		if (a.index() != b.index())
			return false;
		if (std::holds_alternative<std::monostate>(a))
			return true;
		if (const auto* ba = std::get_if<bool>(&a))
			return *ba == *std::get_if<bool>(&b);
		if (const auto* ia = std::get_if<std::int64_t>(&a))
			return *ia == *std::get_if<std::int64_t>(&b);
		if (const auto* da = std::get_if<double>(&a))
			return *da == *std::get_if<double>(&b);
		if (const auto* sa = std::get_if<std::string>(&a))
			return *sa == *std::get_if<std::string>(&b);
		if (const auto* ca = std::get_if<SequenceColorRGBA>(&a))
		{
			const auto* cb = std::get_if<SequenceColorRGBA>(&b);
			return ca->R == cb->R && ca->G == cb->G && ca->B == cb->B && ca->A == cb->A;
		}
		return false;
	}
}
