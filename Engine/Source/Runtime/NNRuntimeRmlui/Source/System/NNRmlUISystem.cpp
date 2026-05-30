/**
 * @file NNRmlUISystem.cpp
 * @brief RmlUI ECS 系统实现——扫描组件、构建 DrawList。
 */

#include "System/NNRmlUISystem.h"
#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"

#include <algorithm>

namespace NN::Runtime::RmlUI
{
	void NNRmlUISystem::Tick(NNRuntimeScene& scene, float deltaTimeSeconds)
	{
		SyncDocuments(scene);
	}

	void NNRmlUISystem::SyncDocuments(NNRuntimeScene& scene)
	{
		// 1. 标记所有快照为 not alive
		for (auto& [entity, snap] : m_Snapshots)
			snap.alive = false;

		// 2. 扫描 ECS，更新快照
		scene.Query<NNRmlUIDocumentComponent>().Each(
			[&](NNEntity entity, NNRmlUIDocumentComponent& comp)
			{
				auto& snap = m_Snapshots[entity];

				// 检测变更
				bool changed = (snap.assetGuid.high != comp.DocumentAsset.high ||
				               snap.assetGuid.low != comp.DocumentAsset.low ||
				               snap.sortOrder != comp.SortOrder ||
				               snap.viewTarget != comp.ViewTarget ||
				               snap.flags != comp.Flags);
				if (changed) m_DrawListDirty = true;

				snap.assetGuid = comp.DocumentAsset;
				snap.sortOrder = comp.SortOrder;
				snap.viewTarget = comp.ViewTarget;
				snap.flags = comp.Flags;
				snap.alive = true;
			});

		// 3. 移除已销毁实体
		for (auto it = m_Snapshots.begin(); it != m_Snapshots.end(); )
		{
			if (!it->second.alive)
			{
				m_DrawListDirty = true;
				it = m_Snapshots.erase(it);
			}
			else
				++it;
		}

		// 4. 重建 DrawList（仅在脏时）
		if (m_DrawListDirty)
		{
			m_DrawList.clear();

			for (const auto& [entity, snap] : m_Snapshots)
			{
				// 跳过未设置资产的文档
				if (snap.assetGuid.high == 0 && snap.assetGuid.low == 0)
					continue;

				RmlDrawItem item;
				item.entity = entity;
				item.assetGuid = snap.assetGuid;
				item.sortOrder = snap.sortOrder;
				item.viewTarget = snap.viewTarget;
				m_DrawList.push_back(item);
			}

			// 按 SortOrder 排序
			std::sort(m_DrawList.begin(), m_DrawList.end(),
				[](const RmlDrawItem& a, const RmlDrawItem& b)
				{ return a.sortOrder < b.sortOrder; });

			m_DrawListDirty = false;
		}
	}

} // namespace NN::Runtime::RmlUI
