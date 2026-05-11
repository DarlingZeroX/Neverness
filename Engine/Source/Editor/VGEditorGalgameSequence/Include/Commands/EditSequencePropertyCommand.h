/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/ISequenceEditorCommand.h"

#include <string>

namespace VisionGal::Editor
{
	enum class SequenceEditFieldId
	{
		CommonDialogue_DialogueText,
		CommonDialogue_CharacterName,
	};

	class EditSequencePropertyCommand final : public ISequenceEditorCommand
	{
	public:
		EditSequencePropertyCommand(unsigned entryIndex, SequenceEditFieldId field, std::string newValue);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		bool TryReadCurrent(SequenceDocument& document, std::string& outCurrent) const;
		void WriteValue(SequenceDocument& document, const std::string& value) const;

		unsigned m_index = 0;
		SequenceEditFieldId m_field = SequenceEditFieldId::CommonDialogue_DialogueText;
		std::string m_oldValue;
		std::string m_newValue;
		bool m_capturedOld = false;
	};
}
