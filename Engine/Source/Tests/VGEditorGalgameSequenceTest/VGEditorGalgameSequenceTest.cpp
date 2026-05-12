/*
 * Minimal tests for sequence document, undo commands, and clipboard.
 */
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "Commands/AddSequenceEntryCommand.h"
#include "Commands/PasteSequenceEntriesCommand.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"
#include "Core/SequenceClipboard.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Async/SequenceAsyncTaskService.h"
#include "Async/SequenceBackgroundValidationTask.h"
#include "Events/SequenceEditorEventBus.h"
#include "Inspector/PropertyEditing/SequencePropertyBindingRegistry.h"
#include "Services/SequenceValidationCacheService.h"
#include "Transactions/SequenceTransactionTypes.h"
#include "Validation/ISequenceValidator.h"
#include "Validation/SequenceValidationRegistriesBootstrap.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"

#include "AssetMonitoring/SequenceDependencyGraph.h"
#include "Events/SequenceEditorEvent.h"
#include "Reactive/SequencePresentationScheduler.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Transactions/SequenceTransactionBuilder.h"
#include "ReactiveCore/DerivedStateGraph.h"
#include "Transactions/Patches/SequenceDocumentPatch.h"
#include "Transactions/Patches/SequencePatchApplier.h"
#include "ViewModels/SequenceSearchViewModel.h"

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
	ASSERT_NE(doc.GetEntryCount(), 0u);
	doc.ResetToUntitledEmpty();
	ASSERT_TRUE(doc.GetAssetPath().empty());
	ASSERT_EQ(doc.GetEntryCount(), 0u);
}

TEST(SequenceUndo, AddThenUndoRestoresCount)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	SequenceUndoStack undo;
	SequenceEditorContext ctx;
	ctx.document = &doc;
	ctx.undo = &undo;

	const unsigned before = doc.GetEntryCount();
	ctx.ExecuteCommand(std::make_unique<AddSequenceEntryCommand>(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()));
	ASSERT_EQ(doc.GetEntryCount(), before + 1u);
	undo.Undo(doc);
	ASSERT_EQ(doc.GetEntryCount(), before);
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

	const unsigned n = doc.GetEntryCount();
	clip.TryPaste(ctx);
	ASSERT_EQ(doc.GetEntryCount(), n + 1u);
}

TEST(PasteSequenceEntriesCommand, UndoRemovesInserted)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	SequenceUndoStack undo;

	std::vector<VisionGal::Ref<VisionGal::IVGSSequenceComponent>> protos;
	protos.push_back(VisionGal::CreateSequenceEntryByTypeNameID(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()));

	undo.ExecuteCommand(std::make_unique<PasteSequenceEntriesCommand>(0, std::move(protos)), doc);
	ASSERT_NE(doc.GetEntryCount(), 0u);
	undo.Undo(doc);
	ASSERT_EQ(doc.GetEntryCount(), 0u);
}

TEST(SequenceDocumentViewModel, RebuildVisibleMatchesSequenceSize)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceComponentRegistry registry;
	BootstrapSequenceComponentRegistry(registry);
	SequenceDocumentViewModel vm;
	vm.Rebuild(doc, registry);
	ASSERT_EQ(vm.GetVisibleEntries().size(), doc.GetEntryCount());
}

TEST(SequenceDocumentViewModel, SearchFilterCanHideAllRows)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceComponentRegistry registry;
	BootstrapSequenceComponentRegistry(registry);
	SequenceDocumentViewModel vm;
	vm.Rebuild(doc, registry);
	vm.ApplySearchFilter("__IMPOSSIBLE_FILTER_TOKEN_XYZ__");
	ASSERT_TRUE(vm.GetVisibleEntries().empty());
}

TEST(SequenceValidationRegistry, BuiltinValidatorsProduceIssuesOnDemoDocument)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceValidationRegistry registry;
	BootstrapSequenceValidationRegistry(registry);
	SequenceComponentRegistry components;
	BootstrapSequenceComponentRegistry(components);
	SequenceDocumentViewModel vm;
	vm.Rebuild(doc, components);
	vm.ApplyValidation(registry, doc);
	ASSERT_FALSE(vm.GetValidationIssues().empty());
}

namespace
{
	class CountingValidator final : public ISequenceValidator
	{
	public:
		mutable int FullValidateCalls = 0;
		mutable int EntryValidateCalls = 0;

		[[nodiscard]] std::vector<SequenceValidationIssue> Validate(const SequenceDocument& document) const override
		{
			(void)document;
			++FullValidateCalls;
			return {};
		}

		[[nodiscard]] std::vector<SequenceValidationIssue> ValidateEntries(
			const SequenceDocument& document,
			const std::vector<unsigned>& entryIndices) const override
		{
			(void)document;
			(void)entryIndices;
			++EntryValidateCalls;
			return {};
		}

		[[nodiscard]] const char* GetRuleId() const override { return "Test.CountingValidator"; }
	};
}

TEST(SequenceEditorEventBus, DocumentChangedDelivery)
{
	SequenceEditorEventBus bus;
	int hits = 0;
	(void)bus.Subscribe(SequenceEditorEventType::DocumentChanged, [&](const SequenceEditorEvent&) { ++hits; });
	SequenceEditorEvent ev;
	ev.Type = SequenceEditorEventType::DocumentChanged;
	bus.Publish(ev);
	ASSERT_EQ(hits, 1);
}

TEST(SequenceDocument, GenerationIncrementsOnContextExecuteCommand)
{
	SequenceDocument doc;
	doc.ResetToUntitledEmpty();
	const uint64_t gen0 = doc.GetGenerationId();
	SequenceUndoStack undo;
	SequenceEditorContext ctx;
	ctx.document = &doc;
	ctx.undo = &undo;
	ctx.ExecuteCommand(std::make_unique<AddSequenceEntryCommand>(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()));
	ASSERT_GT(doc.GetGenerationId(), gen0);
}

TEST(SequenceValidationCacheService, SkipsRepeatedApplyWhenDocumentStable)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceValidationRegistry registry;
	auto counter = std::make_unique<CountingValidator>();
	CountingValidator* raw = counter.get();
	registry.Register(std::move(counter));

	SequenceValidationCacheService cache;
	cache.InvalidateAll();
	ASSERT_TRUE(cache.ApplyIfStale(doc, registry, doc.GetGenerationId()));
	ASSERT_EQ(raw->FullValidateCalls, 1);
	ASSERT_FALSE(cache.ApplyIfStale(doc, registry, doc.GetGenerationId()));
	ASSERT_EQ(raw->FullValidateCalls, 1);
}

TEST(SequenceValidationCacheService, IncrementalPathUsesValidateEntries)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceValidationRegistry registry;
	auto counter = std::make_unique<CountingValidator>();
	CountingValidator* raw = counter.get();
	registry.Register(std::move(counter));

	SequenceValidationCacheService cache;
	cache.InvalidateAll();
	ASSERT_TRUE(cache.ApplyIfStale(doc, registry, doc.GetGenerationId()));
	ASSERT_EQ(raw->FullValidateCalls, 1);

	SequenceDocumentMutationSummary touch;
	touch.StructuralChange = false;
	touch.TouchedIndices.push_back(0);
	cache.NotifyDocumentChanged(touch);
	ASSERT_TRUE(cache.ApplyIfStale(doc, registry, doc.GetGenerationId()));
	ASSERT_EQ(raw->EntryValidateCalls, 1);
	ASSERT_EQ(raw->FullValidateCalls, 1);
}

TEST(SequenceAsyncTaskService, PumpInvokesMergeCallback)
{
	SequenceAsyncTaskService async;
	int merged = 0;
	SequenceBackgroundValidationTask::Schedule(
		async,
		[] { return std::vector<SequenceValidationIssue>{}; },
		[&](std::vector<SequenceValidationIssue> /*issues*/) { ++merged; });
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	async.PumpCompleted();
	ASSERT_EQ(merged, 1);
}

TEST(Phase6_TransactionBuilder, StructuralSummaryProducesStructureMutation)
{
	SequenceDocumentMutationSummary s;
	s.StructuralChange = true;
	const auto tx = BuildTransactionFromMutationSummary(s, 7, SequenceTransactionSource::Command);
	ASSERT_EQ(tx.Generation, 7u);
	ASSERT_FALSE(tx.Mutations.empty());
	ASSERT_EQ(tx.Mutations.front().Type, SequenceMutationType::Structure);
}

TEST(Phase6_DirtyRegion, PropertySummarySetsFlags)
{
	SequenceDocumentMutationSummary s;
	s.StructuralChange = false;
	s.TouchedIndices.push_back(2);
	const auto d = BuildDirtyRegionFromMutationSummary(s);
	ASSERT_NE(d.Flags & SequenceDirtyRegionFlags::Property, SequenceDirtyRegionFlags::None);
	ASSERT_FALSE(d.Entries.empty());
}

TEST(Phase6_PropertyBindingRegistry, CommonDialogueHasBindings)
{
	ASSERT_GE(BuiltinBindingsForCommonDialogue().size(), 2u);
}

TEST(Phase6_DependencyGraph, RebuildFromDemoDocument)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceDependencyGraph graph;
	graph.RebuildFromDocument(doc);
	(void)graph;
}

TEST(Phase6_PresentationScheduler, FirstTickRebuildsViewModel)
{
	SequenceDocument doc;
	doc.FillDefaultDemoEntries();
	SequenceComponentRegistry registry;
	BootstrapSequenceComponentRegistry(registry);
	SequenceDocumentViewModel viewModel;
	SequenceValidationCacheService validationCache;
	SequenceValidationRegistry validationRegistry;
	SequenceSearchIndexService searchIndex;
	SequenceRuntimeObserver runtimeObserver;
	SequenceSearchViewModel searchViewModel;
	SequenceDependencyGraph dependencyGraph;
	SequencePresentationScheduler scheduler;

	bool firstPresentationDone = false;
	SequenceDocumentMutationSummary mutSummary{};
	SequenceDirtyRegion dirty{};

	(void)scheduler.Tick(
		firstPresentationDone,
		mutSummary,
		dirty,
		doc,
		viewModel,
		registry,
		validationCache,
		validationRegistry,
		searchIndex,
		runtimeObserver,
		searchViewModel,
		dependencyGraph,
		nullptr);

	ASSERT_TRUE(firstPresentationDone);
	ASSERT_EQ(viewModel.GetVisibleEntries().size(), doc.GetEntryCount());
}

TEST(Phase8_ReactiveCoreGraph, InvalidatePropagatesDependents)
{
	VisionGal::Editor::ReactiveCore::DerivedStateGraph graph;
	int aRuns = 0;
	int bRuns = 0;
	graph.RegisterNode(1, {}, [&] { ++aRuns; });
	graph.RegisterNode(2, { 1 }, [&] { ++bRuns; });
	graph.Invalidate(1);
	graph.FlushDirty();
	EXPECT_EQ(aRuns, 1);
	EXPECT_EQ(bRuns, 1);
}

TEST(Phase8_PatchTransaction, BuildDirtyRegionFromPatchTransactionStructural)
{
	SequencePatchTransactionV2 tx;
	tx.Patches.push_back(SequenceInsertEntryPatch{ 0u, "X" });
	const SequenceDirtyRegion d = BuildDirtyRegionFromPatchTransaction(tx);
	EXPECT_TRUE(d.StructureChanged);
}
