/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/Builtin/LegacyDrawerInspectorAdapter.h"

#include "Commands/EditSequencePropertyCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Sequence/ComponentDrawerInterface.h"
#include "Sequence/ComponentDrawerRegistry.h"

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"

namespace VisionGal::Editor
{
	LegacyDrawerInspectorAdapter::LegacyDrawerInspectorAdapter(std::string typeNameID, std::string displayName)
		: m_typeNameID(std::move(typeNameID))
		, m_displayName(std::move(displayName))
	{
	}

	void LegacyDrawerInspectorAdapter::OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context)
	{
		if (component == nullptr)
			return;

		const bool commonDialogue = m_typeNameID == VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID();
		if (commonDialogue && context != nullptr && context->document != nullptr && context->undo != nullptr)
		{
			auto* com = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(component);
			if (com == nullptr)
				return;
			if (m_commonStagingIndex != index)
			{
				m_commonStagingIndex = index;
				m_commonStagingName = com->DialogueCharacterName;
				m_commonStagingText = com->DialogueText;
			}

			ImGuiEx::InputText("##DialogueCharacterName", m_commonStagingName);
			if (ImGui::IsItemDeactivatedAfterEdit())
				context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
					index, SequenceEditFieldId::CommonDialogue_CharacterName, m_commonStagingName));

			ImGuiEx::InputTextMultiline(
				"##DialogueText",
				m_commonStagingText,
				ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
			if (ImGui::IsItemDeactivatedAfterEdit())
				context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
					index, SequenceEditFieldId::CommonDialogue_DialogueText, m_commonStagingText));
			return;
		}

		IGalSeqComDrawer* drawer = GalSeqComDrawerRegistry::GetInstance().GetDrawer(m_typeNameID);
		if (drawer == nullptr)
			return;

		drawer->OnGUI(index, component);
	}
}
