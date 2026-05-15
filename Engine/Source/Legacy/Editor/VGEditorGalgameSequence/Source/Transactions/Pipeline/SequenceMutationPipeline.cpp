/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Transactions/Pipeline/SequenceMutationPipeline.h"

#include "Commands/CompoundSequenceCommand.h"
#include "Commands/EditSequencePropertyCommand.h"
#include "Commands/ISequenceEditorCommand.h"
#include "Commands/MoveSequenceEntryCommand.h"
#include "Commands/RemoveSequenceEntryCommand.h"
#include "Commands/SetSequenceEntryBoolPropertyCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "Transactions/Patches/SequenceDocumentPatch.h"
#include "Transactions/Patches/SequencePatchApplier.h"

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"

#include "HCore/Interface/HLog.h"

#include <variant>

namespace VisionGal::Editor
{
	namespace
	{
		class InsertSequenceEntryAtCommand final : public ISequenceEditorCommand
		{
		public:
			InsertSequenceEntryAtCommand(const unsigned atIndex, std::string typeNameId)
				: m_atIndex(atIndex)
				, m_typeNameId(std::move(typeNameId))
			{
			}

			void Execute(SequenceDocument& document) override
			{
				document.InsertEntryByTypeNameID(m_atIndex, m_typeNameId);
				m_insertedIndex = m_atIndex;
			}

			void Undo(SequenceDocument& document) override { document.RemoveEntries({m_insertedIndex}); }

			void Redo(SequenceDocument& document) override { Execute(document); }

			std::string GetDebugName() const override { return "InsertSequenceEntryAtCommand"; }

			[[nodiscard]] SequenceDocumentMutationSummary DescribeExecutedMutation() const override
			{
				SequenceDocumentMutationSummary s;
				s.StructuralChange = true;
				s.TouchedIndices.push_back(m_insertedIndex);
				return s;
			}

		private:
			unsigned m_atIndex = 0;
			std::string m_typeNameId;
			unsigned m_insertedIndex = 0;
		};

		bool TryMapStringPropertyPath(
			const SequenceDocument& document,
			const unsigned entryIndex,
			const std::string& path,
			SequenceEditFieldId& outField)
		{
			const VisionGal::IVGSSequenceComponent* comp = document.GetEntryAt(entryIndex);
			if (comp == nullptr)
				return false;
			if (path == "dialogue" || path.find("DialogueText") != std::string::npos)
			{
				if (dynamic_cast<const VisionGal::VGSSC_CommonDialogue*>(comp) != nullptr)
				{
					outField = SequenceEditFieldId::CommonDialogue_DialogueText;
					return true;
				}
			}
			if (path == "character" || path.find("DialogueCharacterName") != std::string::npos
				|| path.find("CharacterName") != std::string::npos)
			{
				if (dynamic_cast<const VisionGal::VGSSC_CommonDialogue*>(comp) != nullptr)
				{
					outField = SequenceEditFieldId::CommonDialogue_CharacterName;
					return true;
				}
			}
			if (path == "texture" || path.find("TextureResourcePath") != std::string::npos)
			{
				if (dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(comp) != nullptr)
				{
					outField = SequenceEditFieldId::ChangeFigure_TextureResourcePath;
					return true;
				}
				if (dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(comp) != nullptr)
				{
					outField = SequenceEditFieldId::ChangeBackground_TextureResourcePath;
					return true;
				}
			}
			return false;
		}

		bool TryMapBoolPropertyPath(
			const SequenceDocument& document,
			const unsigned entryIndex,
			const std::string& path,
			SequenceEditBoolFieldId& outField)
		{
			const VisionGal::IVGSSequenceComponent* comp = document.GetEntryAt(entryIndex);
			if (comp == nullptr)
				return false;
			if (path == "showState" || path.find("ShowState") != std::string::npos)
			{
				if (dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(comp) != nullptr)
				{
					outField = SequenceEditBoolFieldId::ChangeFigure_ShowState;
					return true;
				}
				if (dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(comp) != nullptr)
				{
					outField = SequenceEditBoolFieldId::ChangeBackground_ShowState;
					return true;
				}
			}
			if (path == "wait" || path.find("Wait") != std::string::npos)
			{
				if (dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(comp) != nullptr)
				{
					outField = SequenceEditBoolFieldId::ChangeFigure_Wait;
					return true;
				}
				if (dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(comp) != nullptr)
				{
					outField = SequenceEditBoolFieldId::ChangeBackground_Wait;
					return true;
				}
			}
			return false;
		}
	}

	void SequenceMutationPipeline::ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command, SequenceEditorContext& ctx)
	{
		if (command == nullptr)
			return;
		SequenceMutationBatch batch;
		batch.Commands.push_back(std::move(command));
		ExecuteBatch(std::move(batch), ctx);
	}

	void SequenceMutationPipeline::ExecuteBatch(SequenceMutationBatch&& batch, SequenceEditorContext& ctx)
	{
		if (batch.Commands.empty())
			return;
		if (ctx.undo == nullptr || ctx.document == nullptr)
		{
			H_LOG_WARN("SequenceMutationPipeline::ExecuteBatch ignored: missing undo stack or document");
			return;
		}
		ctx.undo->ExecuteBatch(std::move(batch), *ctx.document);
		ctx.document->BumpEditGeneration();
		SequenceDocumentMutationSummary summary;
		if (const ISequenceEditorCommand* top = ctx.undo->PeekUndoTop())
			summary = top->DescribeExecutedMutation();
		ctx.NotifyDocumentChanged(summary, SequenceTransactionSource::Command);
	}

	void SequenceMutationPipeline::ExecutePatch(const SequencePatchTransactionV2& tx, SequenceEditorContext& ctx)
	{
		if (ctx.document == nullptr || ctx.undo == nullptr)
			return;
		std::vector<std::unique_ptr<ISequenceEditorCommand>> parts;
		parts.reserve(tx.Patches.size());
		for (const SequenceDocumentPatch& p : tx.Patches)
		{
			if (const auto* ins = std::get_if<SequenceInsertEntryPatch>(&p))
				parts.push_back(std::make_unique<InsertSequenceEntryAtCommand>(ins->AtIndex, ins->TypeNameID));
			else if (const auto* rm = std::get_if<SequenceRemoveEntryPatch>(&p))
				parts.push_back(std::make_unique<RemoveSequenceEntryCommand>(std::vector<unsigned>{rm->Index}));
			else if (const auto* mv = std::get_if<SequenceMoveEntryPatch>(&p))
				parts.push_back(std::make_unique<MoveSequenceEntryCommand>(mv->FromIndex, mv->ToIndex));
			else if (const auto* sp = std::get_if<SequenceSetPropertyPatch>(&p))
			{
				if (const auto* str = std::get_if<std::string>(&sp->Value))
				{
					SequenceEditFieldId field = SequenceEditFieldId::CommonDialogue_DialogueText;
					if (!TryMapStringPropertyPath(*ctx.document, sp->EntryIndex, sp->PropertyPath, field))
					{
						H_LOG_WARN("SequenceMutationPipeline::ExecutePatch: unmapped string property path, skipped");
						continue;
					}
					parts.push_back(std::make_unique<EditSequencePropertyCommand>(sp->EntryIndex, field, *str));
				}
				else if (const auto* b = std::get_if<bool>(&sp->Value))
				{
					SequenceEditBoolFieldId field = SequenceEditBoolFieldId::ChangeFigure_ShowState;
					if (!TryMapBoolPropertyPath(*ctx.document, sp->EntryIndex, sp->PropertyPath, field))
					{
						H_LOG_WARN("SequenceMutationPipeline::ExecutePatch: unmapped bool property path, skipped");
						continue;
					}
					parts.push_back(std::make_unique<SetSequenceEntryBoolPropertyCommand>(sp->EntryIndex, field, *b));
				}
				else
				{
					H_LOG_WARN("SequenceMutationPipeline::ExecutePatch: unsupported property value type, skipped");
					continue;
				}
			}
		}
		if (parts.empty())
			return;
		SequenceMutationBatch batch;
		batch.MergedDirtyRegion = BuildDirtyRegionFromPatchTransaction(tx);
		if (parts.size() == 1)
			batch.Commands.push_back(std::move(parts[0]));
		else
			batch.Commands.push_back(std::make_unique<CompoundSequenceCommand>(std::move(parts)));
		ExecuteBatch(std::move(batch), ctx);
	}
}
