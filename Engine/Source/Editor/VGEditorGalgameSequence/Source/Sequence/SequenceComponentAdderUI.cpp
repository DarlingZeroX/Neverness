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

#include "Sequence/ComponentAdderUI.h"

#include <algorithm>

namespace VisionGal::Editor
{
	namespace
	{
		constexpr float kWideGridThresholdPx = 450.f;
		constexpr float kListSidebarMinWidthPx = 260.f;
	}

	bool ComponentAdderUI::EntryMatchesFilter(const SequenceEntryUIData& entry) const
	{
		if (!m_SearchFilter.IsActive())
			return true;

		std::string blob = entry.Label;
		blob.reserve(blob.size() + entry.FullLabel.size() + entry.TypeNameID.size() + entry.Description.size() + 4);
		blob += ' ';
		blob += entry.FullLabel;
		blob += ' ';
		blob += entry.TypeNameID;
		blob += ' ';
		blob += entry.Description;
		blob += ' ';
		blob += entry.IconLabel;
		return m_SearchFilter.PassFilter(blob.c_str());
	}

	std::vector<CategoryData> ComponentAdderUI::BuildFilteredCategories() const
	{
		std::vector<CategoryData> out;
		out.reserve(m_Categories.size());
		for (const auto& cat : m_Categories)
		{
			CategoryData copy;
			copy.Name = cat.Name;
			copy.TabIcon = cat.TabIcon;
			for (const auto& item : cat.Items)
			{
				if (item.FullLabel.empty() && item.Label.empty())
					continue;
				if (!EntryMatchesFilter(item))
					continue;
				copy.Items.push_back(item);
			}
			out.push_back(std::move(copy));
		}
		return out;
	}

	void ComponentAdderUI::RenderComponentEntryButton(const SequenceEntryUIData& entry)
	{
		const std::string& label = entry.FullLabel.empty()
			? (entry.IconLabel.empty() ? entry.Label : entry.IconLabel + " " + entry.Label)
			: entry.FullLabel;
		if (label.empty())
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.f, 0.5f));
		if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0.f)))
			OnIconClicked.Invoke(entry.TypeNameID);
		ImGui::PopStyleVar();
	}

	ComponentAdderUI::ComponentAdderUI() = default;

	void ComponentAdderUI::SetCategories(std::vector<CategoryData> categories)
	{
		m_ListSelectedCategory = 0;
		m_Categories = std::move(categories);
	}

	void ComponentAdderUI::RenderGridView()
	{
		const std::vector<CategoryData> filtered = BuildFilteredCategories();

		std::vector<int> activeCols;
		activeCols.reserve(filtered.size());
		for (int i = 0; i < static_cast<int>(filtered.size()); ++i)
		{
			if (m_SearchFilter.IsActive())
			{
				if (filtered[static_cast<std::size_t>(i)].Items.empty())
					continue;
			}
			activeCols.push_back(i);
		}

		if (activeCols.empty())
		{
			ImGui::TextUnformatted(u8"无匹配的组件");
			return;
		}

		int maxRows = 0;
		for (int col : activeCols)
			maxRows = std::max(maxRows, static_cast<int>(filtered[static_cast<std::size_t>(col)].Items.size()));

		if (maxRows == 0)
		{
			ImGui::TextUnformatted(u8"无匹配的组件");
			return;
		}

		if (ImGui::BeginChild("SeqComGridScroll", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
		{
			const int cols = static_cast<int>(activeCols.size());
			ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("SequenceComponentTable", cols, tableFlags))
			{
				ImGui::TableNextRow();
				for (int c = 0; c < cols; ++c)
				{
					ImGui::TableSetColumnIndex(c);
					const int srcCol = activeCols[static_cast<std::size_t>(c)];
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f);
					ImGui::TextUnformatted(filtered[static_cast<std::size_t>(srcCol)].Name.c_str());
				}

				for (int row = 0; row < maxRows; ++row)
				{
					ImGui::TableNextRow();
					for (int c = 0; c < cols; ++c)
					{
						ImGui::TableSetColumnIndex(c);
						const int srcCol = activeCols[static_cast<std::size_t>(c)];
						const auto& colItems = filtered[static_cast<std::size_t>(srcCol)].Items;
						if (row >= static_cast<int>(colItems.size()))
							continue;

						const auto& entry = colItems[static_cast<std::size_t>(row)];
						const std::string& label = entry.FullLabel.empty()
							? (entry.IconLabel.empty() ? entry.Label : entry.IconLabel + " " + entry.Label)
							: entry.FullLabel;
						if (label.empty())
							continue;

						if (ImGui::Button(label.c_str()))
							OnIconClicked.Invoke(entry.TypeNameID);
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	void ComponentAdderUI::RenderListView()
	{
		const ImVec2 innerAvail = ImGui::GetContentRegionAvail();
		const bool useCategorySidebar = innerAvail.x >= kListSidebarMinWidthPx;

		const std::vector<CategoryData> filtered = BuildFilteredCategories();

		std::vector<CategoryData> visible;
		visible.reserve(filtered.size());
		for (const auto& cat : filtered)
		{
			if (cat.Items.empty())
				continue;
			visible.push_back(cat);
		}

		if (visible.empty())
		{
			ImGui::TextUnformatted(u8"无匹配的组件");
			return;
		}

		if (m_ListSelectedCategory >= static_cast<int>(visible.size()))
			m_ListSelectedCategory = static_cast<int>(visible.size()) - 1;
		if (m_ListSelectedCategory < 0)
			m_ListSelectedCategory = 0;

		if (ImGui::BeginChild("SeqComListRoot", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
		{
			if (useCategorySidebar)
			{
				const float sidebarW = 96.f;
				if (ImGui::BeginChild("SeqComCatBar", ImVec2(sidebarW, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
				{
					for (int i = 0; i < static_cast<int>(visible.size()); ++i)
					{
						const bool selected = (m_ListSelectedCategory == i);
						const std::string tabLabel = visible[static_cast<std::size_t>(i)].TabIcon + " " + visible[static_cast<std::size_t>(i)].Name;
						if (ImGui::Selectable(tabLabel.c_str(), selected, ImGuiSelectableFlags_None))
							m_ListSelectedCategory = i;
					}
					ImGui::EndChild();
				}
				ImGui::SameLine();
				if (ImGui::BeginChild("SeqComItemScroll", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar))
				{
					const CategoryData& cat = visible[static_cast<std::size_t>(m_ListSelectedCategory)];
					for (const auto& entry : cat.Items)
						RenderComponentEntryButton(entry);
					ImGui::EndChild();
				}
			}
			else
			{
				for (int i = 0; i < static_cast<int>(visible.size()); ++i)
				{
					const CategoryData& cat = visible[static_cast<std::size_t>(i)];
					const std::string header = cat.TabIcon + " " + cat.Name;
					ImGuiTreeNodeFlags flags = (i == 0) ? ImGuiTreeNodeFlags_DefaultOpen : 0;
					if (ImGui::CollapsingHeader(header.c_str(), flags))
					{
						for (const auto& entry : cat.Items)
							RenderComponentEntryButton(entry);
					}
				}
			}
		}
		ImGui::EndChild();
	}

	void ComponentAdderUI::ShowDemoIconsUI()
	{
		ImGui::PushID(this);

		const ImVec2 regionBefore = ImGui::GetContentRegionAvail();
		const bool useWideGrid = regionBefore.x > kWideGridThresholdPx && regionBefore.x > regionBefore.y;
		m_ActiveLayoutMode = useWideGrid ? SequenceComponentLayoutMode::WideGrid : SequenceComponentLayoutMode::UeStyleList;

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(ICON_FA_SEARCH);
		ImGui::SameLine();
		m_SearchFilter.Draw("##SeqComponentSearch", -FLT_MIN);

		if (useWideGrid)
			RenderGridView();
		else
			RenderListView();

		ImGui::PopID();
	}
}
