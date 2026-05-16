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
#include <NNCore/Interface/HConfig.h>
#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeCore/Include/Core/Window.h>
#include <NNEngineLegacy/Include/Render/Texture2D.h>

namespace NN::Editor
{
	struct VG_EDITOR_FRAMEWORK_API EditorMenuBar : public IEditorPanel
	{
		EditorMenuBar() = default;
		EditorMenuBar(Runtime::VGWindow* window);
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
		Runtime::VGWindow* m_EditorWindow = nullptr;
		bool m_EditorMaximized = false;
		Ref<Runtime::Texture2D> m_EngineIcon = nullptr;
	};

}
