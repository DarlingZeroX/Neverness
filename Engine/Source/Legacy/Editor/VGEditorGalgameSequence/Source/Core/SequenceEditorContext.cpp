/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceEditorContext.h"

#include "Commands/ISequenceEditorCommand.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEventBus.h"
#include "Services/SequenceValidationCacheService.h"
#include "Transactions/Pipeline/SequenceMutationPipeline.h"
#include "Transactions/SequenceTransactionBuilder.h"

#include "NNKernel/Interface/HLog.h"

namespace VisionGal::Editor
{
	void SequenceEditorContext::RequestPresentationRefresh()
	{
		if (requestPresentationRefresh != nullptr)
			requestPresentationRefresh(requestPresentationRefreshUserData);
	}

	void SequenceEditorContext::NotifyDocumentChanged(
		const SequenceDocumentMutationSummary& summary,
		const SequenceTransactionSource transactionSource)
	{
		if (validationCache != nullptr)
			validationCache->NotifyDocumentChanged(summary);
		if (onDocumentMutationAccumulate != nullptr)
			onDocumentMutationAccumulate(onDocumentMutationAccumulateUserData, summary);
		if (eventBus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::DocumentChanged;
			ev.DocumentChanged = summary;
			if (document != nullptr)
				ev.CommittedTransaction = BuildTransactionFromMutationSummary(summary, document->GetGenerationId(), transactionSource);
			eventBus->Publish(ev);
		}
		RequestPresentationRefresh();
	}

	void SequenceEditorContext::ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command)
	{
		if (command == nullptr)
			return;
		if (mutationPipeline != nullptr)
		{
			mutationPipeline->ExecuteCommand(std::move(command), *this);
			return;
		}
		if (undo == nullptr || document == nullptr)
		{
			H_LOG_WARN("SequenceEditorContext::ExecuteCommand ignored: missing undo stack or document");
			return;
		}
		undo->ExecuteCommand(std::move(command), *document);
		document->BumpEditGeneration();
		SequenceDocumentMutationSummary summary;
		if (const ISequenceEditorCommand* top = undo->PeekUndoTop())
			summary = top->DescribeExecutedMutation();
		NotifyDocumentChanged(summary, SequenceTransactionSource::Command);
	}

	void SequenceEditorContext::UndoDocument()
	{
		if (undo == nullptr || document == nullptr)
			return;
		undo->Undo(*document);
		document->BumpEditGeneration();
		SequenceDocumentMutationSummary summary;
		summary.StructuralChange = true;
		NotifyDocumentChanged(summary, SequenceTransactionSource::Undo);
	}

	void SequenceEditorContext::RedoDocument()
	{
		if (undo == nullptr || document == nullptr)
			return;
		undo->Redo(*document);
		document->BumpEditGeneration();
		SequenceDocumentMutationSummary summary;
		summary.StructuralChange = true;
		NotifyDocumentChanged(summary, SequenceTransactionSource::Redo);
	}
}
