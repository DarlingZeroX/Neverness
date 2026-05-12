/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace VisionGal::Editor
{
	struct SequenceColorRGBA
	{
		float R = 1.f;
		float G = 1.f;
		float B = 1.f;
		float A = 1.f;
	};

	/// 与 UI / Patch / 运行时观测对齐的值载体（首版覆盖常用标量；可扩展 variant 成员）。
	using SequencePropertyValue = std::variant<
		std::monostate,
		bool,
		std::int64_t,
		double,
		std::string,
		SequenceColorRGBA>;

	[[nodiscard]] bool SequencePropertyValuesEqual(const SequencePropertyValue& a, const SequencePropertyValue& b);
}
