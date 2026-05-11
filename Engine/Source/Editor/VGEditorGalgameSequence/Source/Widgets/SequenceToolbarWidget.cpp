/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceToolbarWidget.h"

#include "Core/SequenceClipboard.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"
#include "Runtime/SequenceExecutionController.h"

#include "HCorePlatform/Include/NativeFileDialog/portable-file-dialogs.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/VFS.h"
#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	namespace
	{
		void RunSaveAsDialog(SequenceDocument& doc)
		{
			const std::string contentPath = VFS::GetInstance()->AbsolutePath(Core::GetAssetsPathVFS());
			std::string root = Horizon::HFileSystem::ToWindowsPath(contentPath);
			const auto destination = pfd::save_file(
					"Save sequence script as...",
					root,
					{ "GalGame Sequence Script (.vgasset)", "*.vgasset" })
					.result();
			if (destination.empty())
				return;
			(void)doc.SaveAsToAssetPath(destination);
		}
	}

	void SequenceToolbarWidget::Render(SequenceEditorContext& ctx)
	{
		if (!ImGui::BeginMenuBar())
			return;

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(u8"新建", nullptr, false, ctx.document != nullptr && ctx.undo != nullptr && ctx.selection != nullptr))
			{
				ctx.undo->Clear();
				ctx.document->ResetToUntitledEmpty();
				ctx.selection->Clear();
			}
			if (ImGui::MenuItem("Save", nullptr, false, ctx.document != nullptr))
			{
				if (ctx.document->GetAssetPath().empty())
					RunSaveAsDialog(*ctx.document);
				else
					(void)ctx.document->SaveToAssetPath();
			}
			if (ImGui::MenuItem(u8"另存为…", nullptr, false, ctx.document != nullptr))
				RunSaveAsDialog(*ctx.document);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			const bool canUndo = ctx.undo != nullptr && ctx.document != nullptr && ctx.undo->CanUndo();
			const bool canRedo = ctx.undo != nullptr && ctx.document != nullptr && ctx.undo->CanRedo();
			if (ImGui::MenuItem("Undo", nullptr, false, canUndo))
				ctx.undo->Undo(*ctx.document);
			if (ImGui::MenuItem("Redo", nullptr, false, canRedo))
				ctx.undo->Redo(*ctx.document);
			ImGui::Separator();
			const bool canCopy = ctx.clipboard != nullptr && ctx.document != nullptr && ctx.selection != nullptr
				&& !ctx.selection->GetSelection().empty();
			const bool canPaste = ctx.clipboard != nullptr && ctx.clipboard->HasContent();
			if (ImGui::MenuItem("Copy", "Ctrl+C", false, canCopy))
				ctx.clipboard->CopySelection(ctx);
			if (ImGui::MenuItem("Cut", "Ctrl+X", false, canCopy))
				ctx.clipboard->CutSelection(ctx);
			if (ImGui::MenuItem("Paste", "Ctrl+V", false, canPaste))
				ctx.clipboard->TryPaste(ctx);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Play"))
		{
			const bool canExec = ctx.document != nullptr && ctx.execution != nullptr && ctx.selection != nullptr
				&& ctx.selection->GetSelection().size() == 1 && ctx.executeToEntry != nullptr;
			if (ImGui::MenuItem(u8"Execute To 选中项", nullptr, false, canExec))
			{
				const unsigned idx = *ctx.selection->GetSelection().begin();
				(void)ctx.executeToEntry(ctx.executeToUserData, idx);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}
