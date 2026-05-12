/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/SetSequenceEntryBoolPropertyCommand.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	SetSequenceEntryBoolPropertyCommand::SetSequenceEntryBoolPropertyCommand(
		unsigned entryIndex,
		SequenceEditBoolFieldId field,
		bool newValue)
		: m_index(entryIndex)
		, m_field(field)
		, m_newValue(newValue)
	{
	}

	bool SetSequenceEntryBoolPropertyCommand::TryReadCurrent(SequenceDocument& document, bool& outCurrent) const
	{
		auto* comp = document.GetEntryAt(m_index);
		if (comp == nullptr)
			return false;
		switch (m_field)
		{
		case SequenceEditBoolFieldId::ChangeFigure_ShowState:
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(comp))
			{
				outCurrent = f->ShowState;
				return true;
			}
			return false;
		case SequenceEditBoolFieldId::ChangeFigure_Wait:
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(comp))
			{
				outCurrent = f->Wait;
				return true;
			}
			return false;
		case SequenceEditBoolFieldId::ChangeBackground_ShowState:
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(comp))
			{
				outCurrent = b->ShowState;
				return true;
			}
			return false;
		case SequenceEditBoolFieldId::ChangeBackground_Wait:
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(comp))
			{
				outCurrent = b->Wait;
				return true;
			}
			return false;
		default:
			return false;
		}
	}

	void SetSequenceEntryBoolPropertyCommand::WriteValue(SequenceDocument& document, bool value) const
	{
		auto* comp = document.GetEntryAt(m_index);
		if (comp == nullptr)
			return;
		switch (m_field)
		{
		case SequenceEditBoolFieldId::ChangeFigure_ShowState:
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(comp))
			{
				f->ShowState = value;
				document.MarkDirty();
			}
			break;
		case SequenceEditBoolFieldId::ChangeFigure_Wait:
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(comp))
			{
				f->Wait = value;
				document.MarkDirty();
			}
			break;
		case SequenceEditBoolFieldId::ChangeBackground_ShowState:
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(comp))
			{
				b->ShowState = value;
				document.MarkDirty();
			}
			break;
		case SequenceEditBoolFieldId::ChangeBackground_Wait:
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(comp))
			{
				b->Wait = value;
				document.MarkDirty();
			}
			break;
		default:
			break;
		}
	}

	void SetSequenceEntryBoolPropertyCommand::Execute(SequenceDocument& document)
	{
		if (!m_capturedOld)
		{
			(void)TryReadCurrent(document, m_oldValue);
			m_capturedOld = true;
		}
		WriteValue(document, m_newValue);
	}

	void SetSequenceEntryBoolPropertyCommand::Undo(SequenceDocument& document)
	{
		WriteValue(document, m_oldValue);
	}

	void SetSequenceEntryBoolPropertyCommand::Redo(SequenceDocument& document)
	{
		WriteValue(document, m_newValue);
	}

	std::string SetSequenceEntryBoolPropertyCommand::GetDebugName() const
	{
		return "SetSequenceEntryBoolPropertyCommand";
	}

	SequenceDocumentMutationSummary SetSequenceEntryBoolPropertyCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = false;
		s.TouchedIndices.push_back(m_index);
		return s;
	}
}
