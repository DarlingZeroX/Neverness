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
#include "../EditorCore/EditorSettingInterface.h"

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API ProjectSettingPanel : public IEditorPanel
	{
	public:
		ProjectSettingPanel() = default;
		ProjectSettingPanel(const ProjectSettingPanel&) = default;
		ProjectSettingPanel& operator=(const ProjectSettingPanel&) = default;
		ProjectSettingPanel(ProjectSettingPanel&&) noexcept = default;
		ProjectSettingPanel& operator=(ProjectSettingPanel&&) noexcept = default;
		~ProjectSettingPanel() override = default;

		void OnGUI() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;

	private:
		bool m_IsOpen = false;
		int m_SelectedPanelIndex = 0;
		EditorSettingInterface* m_SelectedPanel = nullptr;
	};

}
