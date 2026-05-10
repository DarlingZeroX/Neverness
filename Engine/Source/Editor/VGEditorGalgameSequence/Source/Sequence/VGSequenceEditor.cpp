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

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>

#include "Commands/AddSequenceEntryCommand.h"
#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"
#include "VGCore/Include/Core/EventBus.h"
#include "HCorePlatform/Include/NativeFileDialog/portable-file-dialogs.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal::Editor
{
	namespace
	{
		bool RunSaveAsDialog(SequenceDocument& doc)
		{
			const std::string contentPath = VFS::GetInstance()->AbsolutePath(Core::GetAssetsPathVFS());
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

	bool VGScriptSequenceEditor::ExecuteToEntryThunk(void* userData, unsigned index)
	{
		auto* self = static_cast<VGScriptSequenceEditor*>(userData);
		return self->ExecuteTo(index);
	}

	void VGScriptSequenceEditor::SyncContext()
	{
		m_context.document = m_document.get();
		m_context.execution = &m_executionController;
		m_context.selection = &m_selectionModel;
		m_context.undo = &m_undoStack;
		m_context.clipboard = &m_clipboard;
		m_context.inspectorRegistry = &m_inspectorRegistry;
		m_context.searchFilter = &m_searchWidget.GetFilter();
		m_context.executeToEntry = &VGScriptSequenceEditor::ExecuteToEntryThunk;
		m_context.executeToUserData = this;
		m_context.lastExecutionSnapshot = &m_lastRuntimeSnapshot;
	}

	void VGScriptSequenceEditor::InitializeChrome()
	{
		BootstrapSequenceComponentRegistry(m_componentRegistry);
		BootstrapSequenceInspectorRegistry(m_inspectorRegistry, m_componentRegistry);
		m_paletteWidget.ReloadFromRegistry(m_componentRegistry);
		SyncContext();
		m_paletteWidget.OnComponentChosen.Subscribe([this](const std::string& typeNameID) {
			SyncContext();
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
		return m_document->LoadFromAssetPath(path);
	}

	bool VGScriptSequenceEditor::SaveAsset()
	{
		return m_document->SaveToAssetPath();
	}

	VGScriptSequenceEditor::~VGScriptSequenceEditor() = default;

	bool VGScriptSequenceEditor::ExecuteTo(unsigned int index)
	{
		(void)m_document->SaveToAssetPath();
		m_lastRuntimeSnapshot = SequenceRuntimeSnapshot{};
		return m_executionController.ExecuteTo(m_document->GetAssetPath(), index, m_lastRuntimeSnapshot);
	}

	void VGScriptSequenceEditor::RenderSequenceUI()
	{
		SyncContext();
		m_searchWidget.Render(m_context);
		m_entryListWidget.Render(m_context);
		m_inspectorWidget.Render(m_context);
	}

	void VGScriptSequenceEditor::HandleEditorShortcuts()
	{
		SyncContext();
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
		SyncContext();
		m_toolbarWidget.Render(m_context);

		if (ImGui::CollapsingHeader(u8"组件调色板", ImGuiTreeNodeFlags_DefaultOpen))
			m_paletteWidget.Render();

		RenderSequenceUI();
		HandleEditorShortcuts();

		HandleDirtyClosePopup(taskContext);
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

