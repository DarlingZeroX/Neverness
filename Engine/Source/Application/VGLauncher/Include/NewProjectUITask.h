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
#include <VGImgui/IncludeImGuiEx.h>
#include <VGEngine/Include/Core/Core.h>
#include <HFileSystem/Interface/HFileSystem.h>

namespace VisionGal::Editor
{
	class NewProjectUITask : public ImGuiEx::ImTaskInterface
	{
	public:
		NewProjectUITask();
		NewProjectUITask(const NewProjectUITask&) = default;
		NewProjectUITask& operator=(const NewProjectUITask&) = default;
		NewProjectUITask(NewProjectUITask&&) noexcept = default;
		NewProjectUITask& operator=(NewProjectUITask&&) noexcept = default;
		~NewProjectUITask() override = default;

		void RenderUI(TaskContext& context) override;
	private:
		// 打开项目位置选择对话框
		bool OpenProjectLocationDialog();

		// 创建项目并弹出通知
		void CreateProject();

		static void TipText(const std::string& tip);

		String m_Title;
		String m_Text;

		std::string m_ProjectName;
		std::string m_ProjectLocation;
	};

}
