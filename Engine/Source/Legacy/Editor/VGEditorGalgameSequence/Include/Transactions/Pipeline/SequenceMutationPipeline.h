/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "Transactions/Pipeline/SequenceMutationBatch.h"

#include <memory>

namespace VisionGal::Editor
{
	class ISequenceEditorCommand;
	class SequenceEditorContext;
	struct SequencePatchTransactionV2;

	/// Phase 9：命令 / Patch / 事务的统一提交入口；内部委托 `SequenceUndoStack::ExecuteBatch`
	/// 与 `NotifyDocumentChanged`，避免 Context 与撤销栈双轨分裂。
	class SequenceMutationPipeline
	{
	public:
		void ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command, SequenceEditorContext& ctx);

		void ExecuteBatch(SequenceMutationBatch&& batch, SequenceEditorContext& ctx);

		/// 将 Patch v2 转为命令批次并提交（无法映射的 Patch 项会被跳过）。
		void ExecutePatch(const SequencePatchTransactionV2& tx, SequenceEditorContext& ctx);
	};
}
