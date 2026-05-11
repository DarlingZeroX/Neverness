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
#pragma once

#include <memory>
#include <string>

#include "VGEGSExport.h"
#include "Async/SequenceAsyncTaskService.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Core/SequenceClipboard.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEvent.h"
#include "Events/SequenceEditorEventBus.h"
#include "Inspector/SequenceInspectorRegistry.h"
#include "Runtime/SequenceExecutionController.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Runtime/SequenceRuntimeSession.h"
#include "Runtime/SequenceRuntimeSnapshot.h"
#include "Services/SequenceEditorServiceLocator.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Timeline/SequenceTimelineWidget.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "Widgets/SequenceComponentPaletteWidget.h"
#include "Widgets/SequenceEntryListWidget.h"
#include "Widgets/SequenceInspectorWidget.h"
#include "Widgets/SequenceOutlinerWidget.h"
#include "Widgets/SequenceSearchWidget.h"
#include "Widgets/SequenceStatusBarWidget.h"
#include "Widgets/SequenceToolbarWidget.h"
#include "Widgets/SequenceValidationWidget.h"
#include "VGEditorFramework/Interface/UITaskInterface.h"

namespace VisionGal::Editor
{
	class VG_EDITOR_GALGAME_SEQUENCE_API VGScriptSequenceEditor : public IEditorTaskPanel
	{
	public:
		VGScriptSequenceEditor();
		VGScriptSequenceEditor(const std::string& path);
		~VGScriptSequenceEditor() override;

		bool SaveAsset();
		bool OpenAsset(const std::string& path);

		/// Same editor chrome as the task panel, for hosts that embed the editor (e.g. `VisualGalEditor`).
		/// 与任务面板相同的编辑器外壳，供嵌入编辑器的宿主使用（例如 `VisualGalEditor`）。
		void RenderEmbeddedUI();

		void RenderSequenceUI();
		void RenderUI(TaskContext& context) override;

		bool ExecuteTo(unsigned int index);

	private:
		void InitializeChrome();
		void TickEditorPresentation();
		void FillContextPointers();
		void MergePendingDocumentMutation(const SequenceDocumentMutationSummary& summary);

		static void AccumulateDocMutationThunk(void* userData, const SequenceDocumentMutationSummary& summary);
		static void RequestPresentationRefreshThunk(void* userData);

		static bool ExecuteToEntryThunk(void* userData, unsigned index);
		void RenderEditorBody(TaskContext* taskContext);
		void HandleDirtyClosePopup(TaskContext* taskContext);
		void HandleEditorShortcuts();

		std::unique_ptr<SequenceDocument> m_document;
		SequenceDocumentViewModel m_documentViewModel;
		SequenceComponentRegistry m_componentRegistry;
		SequenceInspectorRegistry m_inspectorRegistry;
		SequenceValidationRegistry m_validationRegistry;
		SequenceValidationCacheService m_validationCache;
		SequenceSearchIndexService m_searchIndex;
		SequenceRuntimeObserver m_runtimeObserver;
		SequenceExecutionController m_executionController;
		SequenceRuntimeSession m_runtimeSession;
		SequenceEditorEventBus m_eventBus;
		SequenceEditorServiceLocator m_serviceLocator{};
		SequenceAsyncTaskService m_asyncTaskService;
		SequenceComponentPaletteWidget m_paletteWidget;
		SequenceSelectionModel m_selectionModel;
		SequenceUndoStack m_undoStack;
		SequenceClipboard m_clipboard;
		SequenceEditorContext m_context{};
		SequenceEntryListWidget m_entryListWidget;
		SequenceInspectorWidget m_inspectorWidget;
		SequenceToolbarWidget m_toolbarWidget;
		SequenceSearchWidget m_searchWidget;
		SequenceTimelineWidget m_timelineWidget;
		SequenceOutlinerWidget m_outlinerWidget;
		SequenceValidationWidget m_validationWidget;
		SequenceStatusBarWidget m_statusBarWidget;

		bool m_windowOpen = true;
		SequenceRuntimeSnapshot m_lastRuntimeSnapshot{};

		bool m_needsPresentationTick = true;
		bool m_firstPresentationDone = false;
		SequenceDocumentMutationSummary m_pendingDocMutation{};
	};
}
