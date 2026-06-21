/**
 * @file NNRmlUISystem.cpp
 * @brief RmlUI 系统实现——接收外部 DrawItem 列表，构建 DrawList。
 *
 * 已移除：ECS Query（NNRuntimeScene 移至 Legacy）。
 * 现在由外部调用方（C# RenderCommands）直接传入 DrawItem 列表。
 */

#include "System/NNRmlUISystem.h"

#include <algorithm>

namespace NN::Runtime::RmlUI
{
	void NNRmlUISystem::Tick(float deltaTimeSeconds)
	{
		// 现在由 SetDrawItems 驱动，Tick 只负责排序
		if (m_DrawListDirty)
		{
			std::sort(m_DrawList.begin(), m_DrawList.end(),
				[](const RmlDrawItem& a, const RmlDrawItem& b)
				{ return a.sortOrder < b.sortOrder; });

			m_DrawListDirty = false;
		}
	}

	void NNRmlUISystem::SetDrawItems(const std::vector<RmlDrawItem>& items)
	{
		m_DrawList = items;
		m_DrawListDirty = true;
	}

} // namespace NN::Runtime::RmlUI
