/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertyType.h"

#include <string>

namespace VisionGal::Editor
{
	enum class SequencePortDirection : uint8_t
	{
		Input = 0,
		Output,
	};

	/// 图钉数据类型（可与 SequencePropertyType 对齐；独立枚举便于图执行语义扩展）。
	enum class SequencePortDataType : uint8_t
	{
		Flow = 0,
		Bool,
		Int,
		Float,
		String,
		Object,
	};

	/// 单端口描述（Branch / VisualScript 等图运行时前置数据）。
	struct SequenceGraphPortSchema
	{
		std::string Name;
		std::string DisplayName;
		SequencePortDirection Direction = SequencePortDirection::Input;
		SequencePortDataType DataType = SequencePortDataType::Flow;
		bool MultipleConnections = false;
	};
}
