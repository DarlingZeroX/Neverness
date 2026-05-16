/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>

namespace VisionGal::Editor
{
	enum class SequencePropertyFlags : uint32_t
	{
		None = 0,
		/// 属性必须在语义上存在（结合类型解释，如字符串非空）。
		Required = 1u << 0,
		/// 字符串 / 文本类：去空白后不得为空。
		NotEmpty = 1u << 1,
		/// 资源路径类：路径字符串非空（是否存在文件由宿主校验器补充）。
		ResourcePathNotEmpty = 1u << 2,
		/// 整型 / 浮点：适用 SequencePropertyRange。
		UseRange = 1u << 3,
	};

	constexpr SequencePropertyFlags operator|(SequencePropertyFlags a, SequencePropertyFlags b)
	{
		return static_cast<SequencePropertyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	constexpr SequencePropertyFlags operator&(SequencePropertyFlags a, SequencePropertyFlags b)
	{
		return static_cast<SequencePropertyFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	constexpr bool EnumHasAny(SequencePropertyFlags value, SequencePropertyFlags mask)
	{
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(mask)) != 0u;
	}
}
