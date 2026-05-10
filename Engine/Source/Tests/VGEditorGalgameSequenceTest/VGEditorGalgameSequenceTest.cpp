/*
 * Minimal tests for sequence document, undo commands, and clipboard.
 */
#include <gtest/gtest.h>

#include "Commands/AddSequenceEntryCommand.h"
#include "Commands/PasteSequenceEntriesCommand.h"
#include "Core/SequenceClipboard.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

using namespace VisionGal::Editor;

TEST(SequenceDocument, SaveAsRejectsEmptyPath)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	ASSERT_FALSE(doc.SaveAsToAssetPath(""));
}

TEST(SequenceDocument, ResetToUntitledEmptyClearsSequence)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	ASSERT_FALSE(doc.GetSequence()->m_Sequence.empty());
	doc.ResetToUntitledEmpty();
	ASSERT_TRUE(doc.GetAssetPath().empty());
	ASSERT_TRUE(doc.GetSequence()->m_Sequence.empty());
}

TEST(SequenceUndo, AddThenUndoRestoresCount)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	SequenceUndoStack undo;
	SequenceEditorContext ctx;
	ctx.document = &doc;
	ctx.undo = &undo;

	const size_t before = doc.GetSequence()->m_Sequence.size();
	ctx.ExecuteCommand(std::make_unique<AddSequenceEntryCommand>(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()));
	ASSERT_EQ(doc.GetSequence()->m_Sequence.size(), before + 1);
	undo.Undo(doc);
	ASSERT_EQ(doc.GetSequence()->m_Sequence.size(), before);
}

TEST(SequenceClipboard, DeepCopyPasteIncreasesCount)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	doc.AddEntryByTypeNameID(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID());
	doc.ClearDirty();

	SequenceUndoStack undo;
	SequenceSelectionModel sel;
	SequenceClipboard clip;
	SequenceEditorContext ctx;
	ctx.document = &doc;
	ctx.undo = &undo;
	ctx.selection = &sel;
	ctx.clipboard = &clip;

	sel.SelectSingle(0);
	clip.CopySelection(ctx);
	ASSERT_TRUE(clip.HasContent());

	const size_t n = doc.GetSequence()->m_Sequence.size();
	clip.TryPaste(ctx);
	ASSERT_EQ(doc.GetSequence()->m_Sequence.size(), n + 1);
}

TEST(PasteSequenceEntriesCommand, UndoRemovesInserted)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	SequenceUndoStack undo;

	std::vector<VisionGal::Ref<VisionGal::IVGSSequenceComponent>> protos;
	protos.push_back(VisionGal::CreateSequenceEntryByTypeNameID(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()));

	undo.ExecuteCommand(std::make_unique<PasteSequenceEntriesCommand>(0, std::move(protos)), doc);
	ASSERT_FALSE(doc.GetSequence()->m_Sequence.empty());
	undo.Undo(doc);
	ASSERT_TRUE(doc.GetSequence()->m_Sequence.empty());
}
