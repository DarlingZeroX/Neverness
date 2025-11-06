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
#include <vector>
#include <HCore/Include/HConfig.h>
#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API EditorMenuBar : public IEditorPanel
	{
		EditorMenuBar() = default;
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
	};

}
