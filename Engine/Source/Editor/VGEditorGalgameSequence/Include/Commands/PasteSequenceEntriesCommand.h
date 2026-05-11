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
	/// Inserts deep-copied sequence entries at `insertIndex` (each prototype cloned on Execute/Redo).
	/// 在 `insertIndex` 处插入深拷贝的序列条目（每个原型在 Execute/Redo 时克隆）。
	class PasteSequenceEntriesCommand final : public ISequenceEditorCommand
	{
	public:
		PasteSequenceEntriesCommand(unsigned insertIndex, std::vector<Ref<VisionGal::IVGSSequenceComponent>> prototypes);

		void Execute(SequenceDocument& document) override;
		void Undo(SequenceDocument& document) override;
		void Redo(SequenceDocument& document) override;
		std::string GetDebugName() const override;
		[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override;

	private:
		void DoInsert(SequenceDocument& document);

		unsigned m_insertIndex = 0;
		std::vector<Ref<VisionGal::IVGSSequenceComponent>> m_prototypes;
		unsigned m_insertedCount = 0;
	};
}
