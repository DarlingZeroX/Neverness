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
#include <HCore/Interface/HConfig.h>
#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API EditorSideBar : public IEditorPanel
	{
	public:
		EditorSideBar() = default;
		EditorSideBar(const EditorSideBar&) = default;
		EditorSideBar& operator=(const EditorSideBar&) = default;
		EditorSideBar(EditorSideBar&&) noexcept = default;
		EditorSideBar& operator=(EditorSideBar&&) noexcept = default;
		~EditorSideBar() override = default;

		void OnGUI() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
		void AddComponent(const Ref<ISidebarComponent>& component);
	private:
		void DrawDownStatusBar();
	private:
		//std::vector<Ref<IPanel>> m_Panels;
		std::vector<Ref<ISidebarComponent>> m_Components;
	};

}
