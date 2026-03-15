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
#include "../../Config.h"
#include "VGCore/Include/Core/Core.h"
#include "CodeDocument.h"
#include "DocumentManager.h"

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API CodeStudioPanel : public IEditorPanel
	{
	public:
		CodeStudioPanel();
		~CodeStudioPanel() override;

		void OnGUI() override;
		bool OpenTextFile(const VGPath& path);

		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
	private:
		void DisplayDocContextMenu(CodeDocument* doc);

		void RenderTextEditorUI();
		void RenderFileListUI();

		void DisplayClosingConfirmationUI();

		DocumentManager m_DocManager;
		bool m_IsOpen = false;
	};
}
