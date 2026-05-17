#pragma once

/**
 * @file NNDirtyTracker.h
 * @brief 场景脏状态跟踪（Editor 同步 / 增量序列化预留）。
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Reflection/NNComponentTypeId.h"
#include "Scene/NNEntity.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 跟踪实体级与组件级脏标记；**Consume** 方法取出并清除对应集合。
	 */
	class NN_RUNTIME_SCENE_API NNDirtyTracker
	{
	public:
		void MarkEntityDirty(NNEntity entity) noexcept;

		void MarkComponentDirty(NNEntity entity, NNComponentTypeId typeId) noexcept;

		/** @brief 取出本帧脏实体列表并清空实体脏集。 */
		[[nodiscard]] std::vector<NNEntity> ConsumeDirtyEntities() noexcept;

		/** @brief 取出指定实体的脏组件 TypeId 并自映射中移除该实体项。 */
		[[nodiscard]] std::vector<NNComponentTypeId> ConsumeDirtyComponents(NNEntity entity) noexcept;

		void Clear() noexcept;

		[[nodiscard]] bool IsEntityDirty(NNEntity entity) const noexcept;

	private:
		std::unordered_set<NNEntity> m_DirtyEntities{};
		std::unordered_map<NNEntity, std::unordered_set<NNComponentTypeId>> m_DirtyComponents{};
	};
} // namespace NN::Runtime::Scene
