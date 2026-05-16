/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequenceGraphPortSchema.h"
#include "Schema/SequencePropertySchema.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	/// 组件类型级 Schema：调色板、Inspector、图节点、端口与校验的单一数据源。
	struct SequenceComponentSchema
	{
		std::string TypeNameID;
		std::string DisplayName;
		std::string Description;
		/// 稳定的调色板分组键（对用户显示）。
		std::string Category;
		/// FontAwesome UTF-8 字形（一个或多个）。
		std::string Icon;
		int Priority = 0;

		std::vector<SequencePropertySchema> Properties;

		/// Phase 10-D：图执行 / Branch 预留；线性序列阶段可为空。
		std::vector<SequenceGraphPortSchema> InputPorts;
		std::vector<SequenceGraphPortSchema> OutputPorts;

		[[nodiscard]] std::string PrimaryLabel() const
		{
			return DisplayName.empty() ? TypeNameID : DisplayName;
		}

		[[nodiscard]] std::string PaletteButtonText() const
		{
			const std::string primary = PrimaryLabel();
			if (Icon.empty())
				return primary;
			if (primary.empty())
				return Icon;
			return Icon + " " + primary;
		}
	};
}
