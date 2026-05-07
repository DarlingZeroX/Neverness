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

#include "Sequence/SequenceComponentTableUI.h"

namespace VisionGal::Editor
{
	SequenceComponentUI::SequenceComponentUI()
	{
		m_Headers = { "常规演出", "舞台对象控制", "特殊演出", "场景与分支", "赏鉴", "游戏控制" };
		m_IconLabels.resize(m_Headers.size());

		m_IconLabels[0].push_back(SequenceEntryUIDataManager::GetDataByTypeNameID(VGSSC_CommonDialogue::StaticGetTypeNameID()));
		m_IconLabels[0].push_back(SequenceEntryUIDataManager::GetDataByTypeNameID(VGSSC_ChangeFigure::StaticGetTypeNameID()));

		const char* icons[][4] = {
			{ ICON_FA_COMMENT_ALT " 普通对话", ICON_FA_MUSIC " 背景音乐", ICON_FA_IMAGES " 切换背景", ICON_FA_FILM " 播放视频" },
			{ ICON_FA_EXCHANGE_ALT " 设置动画", ICON_FA_RUNNING " 进出场动画", ICON_FA_MAGIC " 效果与变换", nullptr },
			{ ICON_FA_STAR " 全屏文字", ICON_FA_WAND_MAGIC " 使用特效", ICON_FA_USER_CIRCLE " 角落头像", ICON_FA_ERASER " 清除特效" },
			{ ICON_FA_PROJECT_DIAGRAM " 调用场景", ICON_FA_EXCHANGE_ALT " 切换场景", ICON_FA_CODE_BRANCH " 分支选择", nullptr },
			{ ICON_FA_IMAGES " 赏鉴图片", ICON_FA_MUSIC " 赏鉴音乐", nullptr, nullptr },
			{ ICON_FA_FILE_ALT " 文本显示", ICON_FA_SIGN_OUT_ALT " 结束游戏", nullptr, nullptr }
		};
	}

	void SequenceComponentUI::ShowDemoIconsUI()
	{
		const int rows = 4;
		const int cols = m_Headers.size();

		if (ImGui::BeginTable("SequenceComponentTable", cols, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter))
		{
			// Header
			ImGui::TableNextRow();
			for (int col = 0; col < cols; ++col)
			{
				ImGui::TableSetColumnIndex(col);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
				ImGui::TextUnformatted(m_Headers[col].c_str());
			}
			// Content
			for (int row = 0; row < rows; ++row)
			{
				ImGui::TableNextRow();
				for (int col = 0; col < cols; ++col)
				{
					ImGui::TableSetColumnIndex(col);
					if (row >= m_IconLabels[col].size())
						continue;

					const auto& entry = m_IconLabels[col][row];
					const std::string& label = entry.FullLabel;
					if (label.empty())
						continue;

					if (ImGui::Button(label.c_str(), ImVec2(140, 0)))
					{
						OnIconClicked.Invoke(entry.TypeNameID);
					}
				}
			}
			ImGui::EndTable();
		}

	}
}

