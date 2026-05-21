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

#include "UITask/MessageUITask.h"
#include "EditorCore/Localization.h"

namespace NN::Editor {

	MessageUITask::MessageUITask(Runtime::String const& title, Runtime::String const& text)
		:m_Title(title),m_Text(text)
	{

	}

	void MessageUITask::SetChoices(const std::vector<Runtime::String>& choices)
	{
		m_Choices = choices;
	}

	void MessageUITask::SetCallback(const std::function<void(int)>& callback)
	{
		m_Callback = callback;
	}

	void MessageUITask::RenderUI(TaskContext& context)
	{
		if (context.ForceStop)
			return;

		ImGui::OpenPopup("New Message");

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		bool hasChoice = false;
		int choiceIndex = 0;

		// 确保弹出窗口获得焦点，防止被代码编辑器窗口覆盖
		ImGui::SetNextWindowFocus();
		if (ImGui::BeginPopupModal("New Message", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::Text(EditorText{ m_Title }.c_str());
			ImGui::Text(EditorText{ m_Text }.c_str());

			{	ImGuiEx::ScopedID uiid("New UI Message");

				for (int i = 0; i < m_Choices.size(); ++i)
				{
					choiceIndex = i;
					if (ImGui::Button(EditorText{ m_Choices[i] }.c_str()))
					{
						hasChoice = true;
						break;
					}

					ImGui::SameLine();
				}
			}
			ImGui::EndPopup();
		}

		if (hasChoice)
		{
			context.IsFinished = true;

			if (m_Callback)
			{
				m_Callback(choiceIndex);
			}
		}
	}
}
