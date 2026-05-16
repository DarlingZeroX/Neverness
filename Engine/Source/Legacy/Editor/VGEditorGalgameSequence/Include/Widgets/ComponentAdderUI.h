/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/
#pragma once

#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>
#include <cstdint>
#include <vector>
#include <string>
#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"

namespace VisionGal::Editor
{
	struct CategoryData
	{
		std::string Name;
		/// FontAwesome UTF-8 glyph(s) shown on the category strip / headers
		/// 显示在分类条/标题上的 FontAwesome UTF-8 字形。
		std::string TabIcon;
		std::vector<SequenceComponentMetadata> Items;
	};

	enum class SequenceComponentLayoutMode : std::uint8_t
	{
		WideGrid,
		UeStyleList,
	};

	class ComponentAdderUI
	{
	public:
		ComponentAdderUI();
		~ComponentAdderUI() = default;

		void SetCategories(std::vector<CategoryData> categories);

		void ShowDemoIconsUI();

		Horizon::HEventDelegate<std::string> OnIconClicked;

	private:
		void RenderGridView();
		void RenderListView();

		std::vector<CategoryData> BuildFilteredCategories() const;
		bool EntryMatchesFilter(const SequenceComponentMetadata& entry) const;
		void RenderComponentEntryButton(const SequenceComponentMetadata& entry);

		std::vector<CategoryData> m_Categories;

		ImGuiTextFilter m_SearchFilter;
		SequenceComponentLayoutMode m_ActiveLayoutMode = SequenceComponentLayoutMode::WideGrid;
		int m_ListSelectedCategory = 0;
	};
}
