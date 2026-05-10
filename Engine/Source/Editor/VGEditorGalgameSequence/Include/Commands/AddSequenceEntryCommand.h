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
	class AddSequenceEntryCommand final : public ISequenceEditorCommand
	{
	public:
		explicit AddSequenceEntryCommand(std::string typeNameId);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;

	private:
		void DoAdd(SequenceDocument& document);

		std::string m_typeNameId;
		unsigned m_insertedIndex = 0;
	};
}
