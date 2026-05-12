/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/ISequenceEditorCommand.h"

namespace VisionGal::Editor
{
	class SequenceDocument;

	enum class SequenceEditBoolFieldId
	{
		ChangeFigure_ShowState,
		ChangeFigure_Wait,
		ChangeBackground_ShowState,
		ChangeBackground_Wait,
	};

	class SetSequenceEntryBoolPropertyCommand final : public ISequenceEditorCommand
	{
	public:
		SetSequenceEntryBoolPropertyCommand(unsigned entryIndex, SequenceEditBoolFieldId field, bool newValue);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		bool TryReadCurrent(SequenceDocument& document, bool& outCurrent) const;
		void WriteValue(SequenceDocument& document, bool value) const;

		unsigned m_index = 0;
		SequenceEditBoolFieldId m_field = SequenceEditBoolFieldId::ChangeFigure_ShowState;
		bool m_oldValue = false;
		bool m_newValue = false;
		bool m_capturedOld = false;
	};
}
