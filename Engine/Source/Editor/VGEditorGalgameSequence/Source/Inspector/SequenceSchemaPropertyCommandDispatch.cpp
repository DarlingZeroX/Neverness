/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/SequenceSchemaPropertyCommandDispatch.h"

#include "Commands/EditSequencePropertyCommand.h"
#include "Commands/SetSequenceEntryBoolPropertyCommand.h"
#include "Core/SequenceEditorContext.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"

#include <variant>

namespace VisionGal::Editor
{
	bool TryDispatchSchemaPropertyEdit(
		const std::string& typeNameId,
		const std::string& propertyName,
		const unsigned entryIndex,
		const SequencePropertyValue& value,
		SequenceEditorContext* context)
	{
		if (context == nullptr || context->document == nullptr || context->undo == nullptr)
			return false;

		if (typeNameId == VGSSC_CommonDialogue::StaticGetTypeNameID())
		{
			if (const auto* s = std::get_if<std::string>(&value))
			{
				if (propertyName == "character")
				{
					context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
						entryIndex, SequenceEditFieldId::CommonDialogue_CharacterName, *s));
					return true;
				}
				if (propertyName == "dialogue")
				{
					context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
						entryIndex, SequenceEditFieldId::CommonDialogue_DialogueText, *s));
					return true;
				}
			}
			return false;
		}

		if (typeNameId == VGSSC_ChangeFigure::StaticGetTypeNameID())
		{
			if (propertyName == "texture")
			{
				if (const auto* s = std::get_if<std::string>(&value))
				{
					context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
						entryIndex, SequenceEditFieldId::ChangeFigure_TextureResourcePath, *s));
					return true;
				}
				return false;
			}
			if (const auto* b = std::get_if<bool>(&value))
			{
				if (propertyName == "showState")
				{
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						entryIndex, SequenceEditBoolFieldId::ChangeFigure_ShowState, *b));
					return true;
				}
				if (propertyName == "wait")
				{
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						entryIndex, SequenceEditBoolFieldId::ChangeFigure_Wait, *b));
					return true;
				}
			}
			return false;
		}

		if (typeNameId == VGSSC_ChangeBackground::StaticGetTypeNameID())
		{
			if (propertyName == "texture")
			{
				if (const auto* s = std::get_if<std::string>(&value))
				{
					context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
						entryIndex, SequenceEditFieldId::ChangeBackground_TextureResourcePath, *s));
					return true;
				}
				return false;
			}
			if (const auto* b = std::get_if<bool>(&value))
			{
				if (propertyName == "showState")
				{
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						entryIndex, SequenceEditBoolFieldId::ChangeBackground_ShowState, *b));
					return true;
				}
				if (propertyName == "wait")
				{
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						entryIndex, SequenceEditBoolFieldId::ChangeBackground_Wait, *b));
					return true;
				}
			}
			return false;
		}

		return false;
	}
}
