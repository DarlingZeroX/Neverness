/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "Widgets/ComponentAdderUI.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	/// Owns registered component metadata and can build UI category buckets for the palette.
	/// 持有已注册组件元数据，并可构建调色板用的 UI 分类桶。
	class SequenceComponentRegistry
	{
	public:
		void Register(SequenceComponentMetadata metadata);
		void Unregister(const std::string& typeNameID);

		const SequenceComponentMetadata* Find(const std::string& typeNameID) const;

		/// Stable ordering: ascending Priority, then TypeNameID.
		/// 稳定排序：按 Priority 升序，其次按 TypeNameID。
		std::vector<SequenceComponentMetadata> EnumerateOrdered() const;

		/// Builds palette columns (including empty categories) from default layout + registered types.
		/// 根据默认布局与已注册类型构建调色板列（含空分类）。
		std::vector<CategoryData> BuildPaletteCategories() const;

	private:
		std::unordered_map<std::string, SequenceComponentMetadata> m_byType;
		std::vector<std::string> m_registrationOrder;
	};
}
