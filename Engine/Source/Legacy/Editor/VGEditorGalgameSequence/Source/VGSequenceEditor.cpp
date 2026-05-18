/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "SequenceEditor.h"

#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>

#include "Commands/AddSequenceEntryCommand.h"
#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Events/SequenceEditorEventBus.h"
#include "Validation/SequenceValidationRegistriesBootstrap.h"
#include "Widgets/SequenceRuntimeBridgePanelWidget.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "NNPlatformCore/Include/NativeFileDialog/portable-file-dialogs.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include "NNRuntimeVFS/Include/VFSService.h"

namespace VisionGal::Editor
{
	namespace
	{
		bool RunSaveAsDialog(SequenceDocument& doc)
		{
			const std::string contentPath = VFSService::GetInstance()->AbsolutePath(Core::GetAssetsPathVFS());
			std::string root = Horizon::HFileSystem::ToWindowsPath(contentPath);
			const auto destination = pfd::save_file(
					"Save sequence script as...",
					root,
					{ "GalGame Sequence Script (.vgasset)", "*.vgasset" })
					.result();
			if (destination.empty())
				return false;
			return doc.SaveAsToAssetPath(destination);
		}
	}

	void VGScriptSequenceEditor::AccumulateDocMutationThunk(void* userData, const SequenceDocumentMutationSummary& summary)
	{
		static_cast<VGScriptSequenceEditor*>(userData)->MergePendingDocumentMutation(summary);
	}

	void VGScriptSequenceEditor::RequestPresentationRefreshThunk(void* userData)
	{
		static_cast<VGScriptSequenceEditor*>(userData)->m_needsPresentationTick = true;
	}

	void VGScriptSequenceEditor::MergePendingDocumentMutation(const SequenceDocumentMutationSummary& summary)
	{
		m_pendingDocMutation.StructuralChange = m_pendingDocMutation.StructuralChange || summary.StructuralChange;
		m_pendingDocMutation.TouchedIndices.insert(
			m_pendingDocMutation.TouchedIndices.end(), summary.TouchedIndices.begin(), summary.TouchedIndices.end());

		const SequenceDirtyRegion chunk = BuildDirtyRegionFromMutationSummary(summary);
		m_pendingDirtyRegion.Merge(chunk);

		if (!summary.StructuralChange && !summary.TouchedIndices.empty())
		{
			m_asyncFullValidationArmed = true;
			m_asyncFullValidationDue = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
		}
	}

	void VGScriptSequenceEditor::PumpDebouncedAsyncFullValidation()
	{
		if (!m_asyncFullValidationArmed)
			return;
		if (std::chrono::steady_clock::now() < m_asyncFullValidationDue)
			return;
		m_asyncFullValidationArmed = false;
		if (m_asyncValidationTaskToken != nullptr)
			m_asyncValidationTaskToken->Cancel();
		m_asyncValidationTaskToken = std::make_shared<SequenceTaskToken>();
		const std::shared_ptr<SequenceTaskToken> token = m_asyncValidationTaskToken;
		const uint64_t gen = m_document->GetGenerationId();
		auto clone = m_document->CloneSequenceDeepForValidation();
		auto* reg = &m_validationRegistry;
		m_asyncTaskService.EnqueueValidation(
			[clone = std::move(clone), reg]() {
				SequenceDocument snapshot(std::move(clone), SequenceDocumentValidationSnapshotTag{});
				return reg->RunAll(snapshot);
			},
			[this, gen, token](std::vector<SequenceValidationIssue> issues) {
				if (token->IsCancelled())
					return;
				if (m_document == nullptr || m_document->GetGenerationId() != gen)
					return;
				m_validationCache.ReplaceIssues(std::move(issues));
				m_documentViewModel.ApplyValidationIssues(m_validationCache.GetIssues());
				if (m_context.eventBus != nullptr)
				{
					SequenceEditorEvent ev;
					ev.Type = SequenceEditorEventType::ValidationUpdated;
					m_context.eventBus->Publish(ev);
				}
				m_needsPresentationTick = true;
			});
	}

	bool VGScriptSequenceEditor::ExecuteToEntryThunk(void* userData, unsigned index)
	{
		auto* self = static_cast<VGScriptSequenceEditor*>(userData);
		return self->ExecuteTo(index);
	}

	void VGScriptSequenceEditor::FillContextPointers()
	{
		m_context.document = m_document.get();
		m_context.execution = &m_executionController;
		m_context.selection = &m_selectionModel;
		m_context.undo = &m_undoStack;
		m_context.clipboard = &m_clipboard;
		m_context.inspectorRegistry = &m_inspectorRegistry;
		m_context.componentRegistry = &m_componentRegistry;
		m_context.documentViewModel = &m_documentViewModel;
		m_context.validationRegistry = &m_validationRegistry;
		m_context.validationCache = &m_validationCache;
		m_context.runtimeOverlay = &m_runtimeObserver.GetOverlay();
		m_context.searchFilter = &m_searchWidget.GetFilter();
		m_context.executeToEntry = &VGScriptSequenceEditor::ExecuteToEntryThunk;
		m_context.executeToUserData = this;
		m_context.lastExecutionSnapshot = &m_lastRuntimeSnapshot;
		m_context.eventBus = &m_eventBus;
		m_context.debuggerSession = &m_debuggerSession;
		m_context.graphProjection = &m_presentationScheduler.GetGraphProjection();
		m_context.services = &m_serviceLocator;
		m_context.authoringGraph = &m_editorSession.GetAuthoringGraph();
		m_context.projectionEventBus = &m_editorSession.GetProjectionEventBus();
		m_context.runtimeEventTimeline = &m_editorSession.GetRuntimeEventTimeline();
		m_context.mutationPipeline = &m_mutationPipeline;
	}

	void VGScriptSequenceEditor::TickEditorPresentation()
	{
		if (!m_needsPresentationTick && m_firstPresentationDone)
			return;

		m_needsPresentationTick = false;

		const SequenceDocumentMutationSummary mut = std::move(m_pendingDocMutation);
		m_pendingDocMutation = SequenceDocumentMutationSummary{};

		SequenceDirtyRegion dirty = std::move(m_pendingDirtyRegion);
		m_pendingDirtyRegion.Reset();

		(void)m_presentationScheduler.Tick(
			m_firstPresentationDone,
			mut,
			dirty,
			*m_document,
			m_documentViewModel,
			m_componentRegistry,
			m_validationCache,
			m_validationRegistry,
			m_searchIndex,
			m_runtimeObserver,
			m_searchWidget.GetSearchViewModel(),
			m_selectionModel,
			m_dependencyGraph,
			&m_eventBus);

		FillContextPointers();
	}

	void VGScriptSequenceEditor::InitializeChrome()
	{
		BootstrapSequenceComponentRegistry(m_componentRegistry);
		BootstrapSequenceValidationRegistry(m_validationRegistry, &m_componentRegistry);
		BootstrapSequenceInspectorRegistry(m_inspectorRegistry, m_componentRegistry);

		m_serviceLocator.validationCache = &m_validationCache;
		m_serviceLocator.searchIndex = &m_searchIndex;
		m_serviceLocator.debuggerSession = &m_debuggerSession;
		m_serviceLocator.asyncTasks = &m_asyncTaskService;

		m_debuggerSession.Bind(
			&m_executionController,
			&m_runtimeObserver,
			&m_eventBus,
			&m_editorSession.GetRuntimeEventTimeline());
		m_assetDependency.Bind(
			m_document.get(),
			&m_dependencyGraph,
			&m_validationCache,
			&VGScriptSequenceEditor::RequestPresentationRefreshThunk,
			this);

		m_selectionModel.SetEventBus(&m_eventBus);

		m_selectionProjectionController.Bind(m_editorSession.GetProjectionEventBus(), m_selectionModel, &m_eventBus);
		BootstrapSequenceExtensions(m_editorSession.GetExtensionRegistry());
		m_editorSession.GetExtensionRegistry().NotifySessionBegin();
		m_presentationScheduler.SetAuthoringGraph(&m_editorSession.GetAuthoringGraph());

		m_editorSession.SetProjectionPipeline(&m_presentationScheduler.GetProjectionPipeline());
		m_editorSession.SetDataConsistencyPipeline(&m_presentationScheduler.GetDataConsistencyPipeline());
		m_editorSession.SetMutationPipeline(&m_mutationPipeline);
		m_editorSession.SetRuntimeKernel(&m_debuggerSession.GetRuntimeKernel());

		m_context.eventBus = &m_eventBus;
		m_context.services = &m_serviceLocator;
		m_context.validationCache = &m_validationCache;
		m_context.onDocumentMutationAccumulate = &VGScriptSequenceEditor::AccumulateDocMutationThunk;
		m_context.onDocumentMutationAccumulateUserData = this;
		m_context.requestPresentationRefresh = &VGScriptSequenceEditor::RequestPresentationRefreshThunk;
		m_context.requestPresentationRefreshUserData = this;

		FillContextPointers();
		m_validationCache.InvalidateAll();
		m_needsPresentationTick = true;
		TickEditorPresentation();

		m_paletteWidget.ReloadFromRegistry(m_componentRegistry);
		m_paletteWidget.OnComponentChosen.Subscribe([this](const std::string& typeNameID) {
			FillContextPointers();
			m_context.ExecuteCommand(std::make_unique<AddSequenceEntryCommand>(typeNameID));
		});
	}

	VGScriptSequenceEditor::VGScriptSequenceEditor()
	{
		m_document = std::make_unique<SequenceDocument>();
		m_document->FillDefaultDemoEntries();
		InitializeChrome();
	}

	VGScriptSequenceEditor::VGScriptSequenceEditor(const std::string& path)
	{
		m_document = std::make_unique<SequenceDocument>();
		(void)m_document->LoadFromAssetPath(path);
		InitializeChrome();

		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& event) {
			if (event.EventType == EngineEventType::EnterScenePlayMode)
				this->SaveAsset();
		});
	}

	bool VGScriptSequenceEditor::OpenAsset(const std::string& path)
	{
		const bool ok = m_document->LoadFromAssetPath(path);
		if (ok)
		{
			m_validationCache.InvalidateAll();
			FillContextPointers();
			SequenceDocumentMutationSummary summary;
			summary.StructuralChange = true;
			m_context.NotifyDocumentChanged(summary);
		}
		return ok;
	}

	bool VGScriptSequenceEditor::SaveAsset()
	{
		const bool ok = m_document->SaveToAssetPath();
		if (ok && m_document != nullptr && !m_document->GetAssetPath().empty())
			m_assetDependency.OnAssetChanged(m_document->GetAssetPath());
		return ok;
	}

	VGScriptSequenceEditor::~VGScriptSequenceEditor()
	{
		m_editorSession.GetExtensionRegistry().NotifySessionEnd();
	}

	bool VGScriptSequenceEditor::ExecuteTo(unsigned int index)
	{
		(void)m_document->SaveToAssetPath();
		m_lastRuntimeSnapshot = SequenceRuntimeSnapshot{};
		const bool ok = m_debuggerSession.RequestRunTo(m_document->GetAssetPath(), index, m_lastRuntimeSnapshot);
		m_needsPresentationTick = true;
		return ok;
	}

	void VGScriptSequenceEditor::RenderSequenceUI()
	{
		FillContextPointers();
		m_searchWidget.Render(m_context);
		m_entryListWidget.Render(m_context);
		m_inspectorWidget.Render(m_context);
	}

	void VGScriptSequenceEditor::HandleEditorShortcuts()
	{
		FillContextPointers();
		if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
			return;
		ImGuiIO& io = ImGui::GetIO();
		if (!io.KeyCtrl)
			return;
		if (ImGui::IsKeyPressed(ImGuiKey_C, false))
			m_clipboard.CopySelection(m_context);
		else if (ImGui::IsKeyPressed(ImGuiKey_X, false))
			m_clipboard.CutSelection(m_context);
		else if (ImGui::IsKeyPressed(ImGuiKey_V, false))
			m_clipboard.TryPaste(m_context);
	}

	void VGScriptSequenceEditor::RenderEditorBody(TaskContext* taskContext)
	{
		m_asyncTaskService.PumpCompleted();
		PumpDebouncedAsyncFullValidation();
		TickEditorPresentation();

		FillContextPointers();
		m_toolbarWidget.Render(m_context);
		ImGui::Separator();
		m_statusBarWidget.Render(m_context);

		ImGuiID classId = ImHashStr("VGScriptSequenceEditor");
		ImGui::DockSpace(classId, ImVec2(0, 0));

		if (m_workspace.IsWindowVisible("Palette"))
		{
			if (ImGui::Begin(u8"组件调色板"))
				m_paletteWidget.Render();
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("Timeline"))
		{
			if (ImGui::Begin(u8"时间轴1"))
				m_timelineWidget.Render(m_context);
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("Outliner"))
		{
			if (ImGui::Begin(u8"大纲"))
				m_outlinerWidget.Render(m_context);
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("Validation"))
		{
			if (ImGui::Begin(u8"校验面板"))
				m_validationWidget.Render(m_context);
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("Sequence"))
		{
			if (ImGui::Begin(u8"序列"))
				RenderSequenceUI();
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("Graph"))
		{
			if (ImGui::Begin(u8"序列图"))
				m_graphWidget.Render(m_context);
			ImGui::End();
		}

		if (m_workspace.IsWindowVisible("RuntimeBridge"))
		{
			if (ImGui::Begin(u8"运行时事件桥"))
				SequenceRuntimeBridgePanelWidget::Render(m_context);
			ImGui::End();
		}

		HandleEditorShortcuts();

		m_asyncTaskService.PumpCompleted();
		PumpDebouncedAsyncFullValidation();
		TickEditorPresentation();
	}

	void VGScriptSequenceEditor::HandleDirtyClosePopup(TaskContext* taskContext)
	{
		if (taskContext == nullptr)
			return;

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("VGSeqUnsavedChanges", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted(u8"当前序列有未保存的更改。关闭前请选择：");
			if (ImGui::Button(u8"保存并关闭", ImVec2(120, 0)))
			{
				bool saved = false;
				if (m_document->GetAssetPath().empty())
					saved = RunSaveAsDialog(*m_document);
				else
					saved = SaveAsset();
				if (saved)
				{
					m_windowOpen = false;
					taskContext->IsFinished = true;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"不保存", ImVec2(120, 0)))
			{
				m_windowOpen = false;
				taskContext->IsFinished = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"取消", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	void VGScriptSequenceEditor::RenderEmbeddedUI()
	{
		if (ImGui::Begin("Visual GalGame Editor", nullptr, ImGuiWindowFlags_MenuBar))
		{
			ImGuiEx::ScopedID scopedId(static_cast<int>(0x4E5647));
			RenderEditorBody(nullptr);
		}
		ImGui::End();
	}

	void VGScriptSequenceEditor::RenderUI(TaskContext& context)
	{

		std::string windowName = "Visual GalGame Editor##" + std::to_string(context.Index);
		if (ImGui::Begin(windowName.c_str(), &m_windowOpen, ImGuiWindowFlags_MenuBar))
		{
			ImGuiEx::ScopedID winID(static_cast<int>(context.Index));
			RenderEditorBody(&context);
			if (!m_windowOpen && m_document->IsDirty())
			{
				m_windowOpen = true;
				ImGui::OpenPopup("VGSeqUnsavedChanges");
			}
		}
		ImGui::End();

		if (!m_windowOpen && !m_document->IsDirty())
			context.IsFinished = true;
	}
}
