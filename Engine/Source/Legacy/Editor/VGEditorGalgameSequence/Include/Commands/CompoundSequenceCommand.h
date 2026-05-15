/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/ISequenceEditorCommand.h"

#include <memory>
#include <vector>

namespace VisionGal::Editor
{
	class CompoundSequenceCommand final : public ISequenceEditorCommand
	{
	public:
		explicit CompoundSequenceCommand(std::vector<std::unique_ptr<ISequenceEditorCommand>> parts);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		std::vector<std::unique_ptr<ISequenceEditorCommand>> m_parts;
	};
}
