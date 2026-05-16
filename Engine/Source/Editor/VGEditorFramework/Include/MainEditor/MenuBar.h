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
#include <vector>
#include <NNKernel/Interface/HConfig.h>
#include <VGImgui/IncludeImGui.h>
#include <VGCore/Include/Core/Window.h>
#include <VGEngine/Include/Render/Texture2D.h>

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API EditorMenuBar : public IEditorPanel
	{
		EditorMenuBar() = default;
		EditorMenuBar(VGWindow* window);
		EditorMenuBar(const EditorMenuBar&) = default;
		EditorMenuBar& operator=(const EditorMenuBar&) = default;
		EditorMenuBar(EditorMenuBar&&) noexcept = default;
		EditorMenuBar& operator=(EditorMenuBar&&) noexcept = default;
		~EditorMenuBar() override = default;

		void OnGUI() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;

	private:
		void HandleDraggingWindow();
		void HandleWindowControl();
		bool m_bDragging = false;
		VGWindow* m_EditorWindow = nullptr;
		bool m_EditorMaximized = false;
		Ref<Texture2D> m_EngineIcon = nullptr;
	};

}
