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
#include "../Config.h"
#include "../EditorComponents/PanelInterface.h"
#include <VGImgui/Include/ImGuiColorTextEdit/TextEditor.h>
#include "VGEngine/Include/Core/Core.h"

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API TextEditorPanel : public IEditorPanel
	{
	public:
		TextEditorPanel();
		~TextEditorPanel() override;

		void OnGUI() override;
		bool OpenTextFile(const VGPath& path);

		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
	private:
		class ShaderEditorList;
		void RenderTextEditorUI();
		void RenderFileListUI();

		void ReadLastWriteTime();

		bool m_bItemDragging;
		bool m_IsTextChanged = false;

		std::set<std::string> m_FileList;
		std::unique_ptr<ShaderEditorList> m_ListPanel;
		//ImGuiTextEditorOriginal::TextEditor m_TexEditor;
		ImGuiTextEditor::TextEditor m_TexEditor;

		std::string m_Text;
		std::string m_CurrentTextPath;
		std::filesystem::file_time_type m_TextFileLastWriteTime;

		bool m_HasText = false;
		bool m_IsOpen = false;
	};
}
