/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceInspectorWidget.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Document/SequenceDocument.h"
#include "Inspector/SequenceAutoInspectorDrawer.h"
#include "Inspector/SequenceInspectorRegistry.h"

#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	void SequenceInspectorWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.document == nullptr || ctx.selection == nullptr || ctx.inspectorRegistry == nullptr)
		{
			ImGui::TextUnformatted(u8"（Inspector 不可用）");
			return;
		}

		const auto& sel = ctx.selection->GetSelection();
		if (sel.empty())
		{
			ImGui::TextUnformatted(u8"请选择序列条目以编辑属性。");
			return;
		}
		if (sel.size() != 1)
		{
			ImGui::TextUnformatted(u8"多选时不显示属性面板。");
			return;
		}

		const unsigned idx = *sel.begin();
		auto* entry = ctx.document->GetEntryAt(idx);
		if (entry == nullptr)
		{
			ImGui::TextUnformatted(u8"选中索引无效。");
			return;
		}

		if (!ctx.inspectorRegistry->DrawInspector(entry->GetTypeNameID(), idx, entry, &ctx))
		{
			if (ctx.componentRegistry != nullptr)
			{
				if (const SequenceComponentMetadata* meta = ctx.componentRegistry->Find(entry->GetTypeNameID()))
				{
					if (TryDrawAutoInspectorFromDescriptors(*meta, idx, entry, &ctx))
						return;
				}
			}
			ImGui::TextUnformatted(u8"（未注册 Inspector）");
		}
	}
}
