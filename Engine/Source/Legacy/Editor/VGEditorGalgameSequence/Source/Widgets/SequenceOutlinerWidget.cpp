/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceOutlinerWidget.h"

#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceEntryViewModel.h"

#include <VGImgui/IncludeImGui.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	void SequenceOutlinerWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.documentViewModel == nullptr || ctx.selection == nullptr)
			return;

		ImGui::TextUnformatted(u8"大纲 (按分类)");
		const auto& rows = ctx.documentViewModel->GetVisibleEntries();
		std::unordered_map<std::string, std::vector<const SequenceEntryViewModel*>> byCategory;
		for (const auto& row : rows)
		{
			const std::string key = row.Category.empty() ? std::string(u8"(未分类)") : row.Category;
			byCategory[key].push_back(&row);
		}

		if (ImGui::BeginChild("SeqOutlinerInner", ImVec2(0, 160), true))
		{
			std::vector<std::string> keys;
			keys.reserve(byCategory.size());
			for (const auto& kv : byCategory)
				keys.push_back(kv.first);
			std::sort(keys.begin(), keys.end());
			for (const std::string& catKey : keys)
			{
				const auto it = byCategory.find(catKey);
				if (it == byCategory.end())
					continue;
				if (ImGui::TreeNodeEx(catKey.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (const SequenceEntryViewModel* p : it->second)
					{
						if (p == nullptr)
							continue;
						ImGui::PushID(static_cast<int>(p->EntryIndex));
						const std::string line = "#" + std::to_string(p->EntryIndex) + " " + p->DisplayName;
						const bool sel = ctx.selection->IsSelected(p->EntryIndex);
						if (ImGui::Selectable(line.c_str(), sel))
						{
							const bool heldCtrl = ImGui::GetIO().KeyCtrl;
							if (heldCtrl)
								ctx.selection->ToggleSelection(p->EntryIndex);
							else
								ctx.selection->SelectSingle(p->EntryIndex);
						}
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild();
	}
}
