/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/SequenceAutoInspectorDrawer.h"

#include "Commands/EditSequencePropertyCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Properties/SequencePropertyDescriptor.h"

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>

#include <cstdint>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <unordered_map>

namespace VisionGal::Editor
{
	namespace
	{
		using StagingKey = std::uint64_t;

		StagingKey MakeKey(const unsigned index, const SequenceEditFieldId field)
		{
			return (static_cast<std::uint64_t>(index) << 32)
				| static_cast<std::uint32_t>(static_cast<int>(field));
		}

		std::unordered_map<StagingKey, std::string>& StagingStrings()
		{
			static std::unordered_map<StagingKey, std::string> s;
			return s;
		}

		bool TryReadField(
			VisionGal::IVGSSequenceComponent* comp,
			const SequenceEditFieldId field,
			std::string& out)
		{
			if (comp == nullptr)
				return false;
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(comp))
			{
				if (field == SequenceEditFieldId::CommonDialogue_DialogueText)
				{
					out = d->DialogueText;
					return true;
				}
				if (field == SequenceEditFieldId::CommonDialogue_CharacterName)
				{
					out = d->DialogueCharacterName;
					return true;
				}
			}
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(comp))
			{
				if (field == SequenceEditFieldId::ChangeFigure_TextureResourcePath)
				{
					out = f->TextureResourcePath;
					return true;
				}
			}
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(comp))
			{
				if (field == SequenceEditFieldId::ChangeBackground_TextureResourcePath)
				{
					out = b->TextureResourcePath;
					return true;
				}
			}
			return false;
		}
	}

	bool TryDrawAutoInspectorFromDescriptors(
		const SequenceComponentMetadata& meta,
		const unsigned index,
		VisionGal::IVGSSequenceComponent* component,
		SequenceEditorContext* context)
	{
		if (component == nullptr || context == nullptr || context->document == nullptr || context->undo == nullptr)
			return false;
		if (meta.PropertyDescriptors.empty())
			return false;

		bool drew = false;
		for (const SequencePropertyDescriptor& d : meta.PropertyDescriptors)
		{
			if (!d.HasEditField
				|| (d.Kind != SequencePropertyKind::String && d.Kind != SequencePropertyKind::AssetRef))
				continue;

			const StagingKey key = MakeKey(index, d.EditField);
			std::string& staging = StagingStrings()[key];
			if (staging.empty())
			{
				if (!TryReadField(component, d.EditField, staging))
					staging.clear();
			}

			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&d) ^ static_cast<int>(index)));
			ImGuiEx::InputText(d.Label.c_str(), staging);
			if (ImGui::IsItemDeactivatedAfterEdit())
				context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(index, d.EditField, staging));
			ImGui::PopID();
			drew = true;
		}
		return drew;
	}
}
