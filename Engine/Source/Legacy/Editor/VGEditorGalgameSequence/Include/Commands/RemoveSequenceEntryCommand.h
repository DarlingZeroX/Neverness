/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/ISequenceEditorCommand.h"

#include <HCore/Interface/HConfig.h>
#include <vector>

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	class RemoveSequenceEntryCommand final : public ISequenceEditorCommand
	{
	public:
		explicit RemoveSequenceEntryCommand(std::vector<unsigned> indices);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		void CaptureSnapshotIfNeeded(SequenceDocument& document);

		std::vector<unsigned> m_indices;
		std::vector<std::pair<unsigned, Ref<VisionGal::IVGSSequenceComponent>>> m_undoPairs;
		bool m_hasSnapshot = false;
	};
}
