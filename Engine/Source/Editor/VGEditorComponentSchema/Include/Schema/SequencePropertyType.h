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
	/// 属性在 Authoring / Inspector / Graph / Patch 中的统一类型枚举（Phase 10）。
	/// 未在本阶段实现 UI 的类型仍可用于元数据预留与校验占位。
	enum class SequencePropertyType : uint8_t
	{
		Unknown = 0,
		Bool,
		Int,
		Float,
		String,
		ResourcePath,
		Enum,
		Color,
		Vector2,
		Vector3,
		Struct,
		Array,
		ObjectReference,
		LocalizedText,
	};
}
