/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/ISequenceEditorCommand.h"

#include <NNKernel/Interface/HConfig.h>
#include <vector>

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	class MoveSequenceEntryCommand final : public ISequenceEditorCommand
	{
	public:
		MoveSequenceEntryCommand(unsigned sourceIndex, unsigned targetIndex);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		void DoReorder(SequenceDocument& document);

		unsigned m_source = 0;
		unsigned m_target = 0;
		std::vector<Ref<VisionGal::IVGSSequenceComponent>> m_beforeOrder;
	};
}
