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
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Core/SequenceClipboard.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "Inspector/SequenceInspectorRegistry.h"
#include "Runtime/SequenceExecutionController.h"
#include "Runtime/SequenceRuntimeSnapshot.h"
#include "Widgets/SequenceComponentPaletteWidget.h"
#include "Widgets/SequenceEntryListWidget.h"
#include "Widgets/SequenceInspectorWidget.h"
#include "Widgets/SequenceSearchWidget.h"
#include "Widgets/SequenceToolbarWidget.h"
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
		void RenderEmbeddedUI();

		void RenderSequenceUI();
		void RenderUI(TaskContext& context) override;

		bool ExecuteTo(unsigned int index);

	private:
		void InitializeChrome();
		void SyncContext();
		static bool ExecuteToEntryThunk(void* userData, unsigned index);
		void RenderEditorBody(TaskContext* taskContext);
		void HandleDirtyClosePopup(TaskContext* taskContext);
		void HandleEditorShortcuts();

		std::unique_ptr<SequenceDocument> m_document;
		SequenceComponentRegistry m_componentRegistry;
		SequenceInspectorRegistry m_inspectorRegistry;
		SequenceExecutionController m_executionController;
		SequenceComponentPaletteWidget m_paletteWidget;
		SequenceSelectionModel m_selectionModel;
		SequenceUndoStack m_undoStack;
		SequenceClipboard m_clipboard;
		SequenceEditorContext m_context{};
		SequenceEntryListWidget m_entryListWidget;
		SequenceInspectorWidget m_inspectorWidget;
		SequenceToolbarWidget m_toolbarWidget;
		SequenceSearchWidget m_searchWidget;

		bool m_windowOpen = true;
		SequenceRuntimeSnapshot m_lastRuntimeSnapshot{};
	};

}
