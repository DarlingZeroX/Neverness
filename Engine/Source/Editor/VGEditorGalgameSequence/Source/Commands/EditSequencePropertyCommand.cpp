/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/EditSequencePropertyCommand.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	EditSequencePropertyCommand::EditSequencePropertyCommand(unsigned entryIndex, SequenceEditFieldId field, std::string newValue)
		: m_index(entryIndex)
		, m_field(field)
		, m_newValue(std::move(newValue))
	{
	}

	bool EditSequencePropertyCommand::TryReadCurrent(SequenceDocument& document, std::string& outCurrent) const
	{
		const auto seq = document.GetSequence();
		if (m_index >= seq->m_Sequence.size())
			return false;
		auto* comp = seq->m_Sequence[m_index].get();
		if (comp == nullptr)
			return false;
		switch (m_field)
		{
		case SequenceEditFieldId::CommonDialogue_DialogueText:
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(comp))
			{
				outCurrent = d->DialogueText;
				return true;
			}
			return false;
		case SequenceEditFieldId::CommonDialogue_CharacterName:
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(comp))
			{
				outCurrent = d->DialogueCharacterName;
				return true;
			}
			return false;
		default:
			return false;
		}
	}

	void EditSequencePropertyCommand::WriteValue(SequenceDocument& document, const std::string& value) const
	{
		const auto seq = document.GetSequence();
		if (m_index >= seq->m_Sequence.size())
			return;
		auto* comp = seq->m_Sequence[m_index].get();
		if (comp == nullptr)
			return;
		switch (m_field)
		{
		case SequenceEditFieldId::CommonDialogue_DialogueText:
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(comp))
			{
				d->DialogueText = value;
				document.MarkDirty();
			}
			break;
		case SequenceEditFieldId::CommonDialogue_CharacterName:
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(comp))
			{
				d->DialogueCharacterName = value;
				document.MarkDirty();
			}
			break;
		default:
			break;
		}
	}

	void EditSequencePropertyCommand::Execute(SequenceDocument& document)
	{
		if (!m_capturedOld)
		{
			(void)TryReadCurrent(document, m_oldValue);
			m_capturedOld = true;
		}
		WriteValue(document, m_newValue);
	}

	void EditSequencePropertyCommand::Undo(SequenceDocument& document)
	{
		WriteValue(document, m_oldValue);
	}

	void EditSequencePropertyCommand::Redo(SequenceDocument& document)
	{
		WriteValue(document, m_newValue);
	}

	std::string EditSequencePropertyCommand::GetDebugName() const
	{
		return "EditSequencePropertyCommand";
	}

	SequenceDocumentMutationSummary EditSequencePropertyCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = false;
		s.TouchedIndices.push_back(m_index);
		return s;
	}
}
