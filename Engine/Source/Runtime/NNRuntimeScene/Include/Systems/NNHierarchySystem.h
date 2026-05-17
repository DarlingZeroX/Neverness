#pragma once

/**
 * @file NNHierarchySystem.h
 * @brief 层级 System：维护父子关系与子列表，同步 **NNRelationshipComponent**。
 */

#include <unordered_map>
#include <vector>

#include "Systems/ISceneSystem.h"
#include "Scene/NNEntity.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 数据导向层级管理；子列表存于 System 侧 map，组件仅保留 parent/depth/childCount。
	 */
	class NN_RUNTIME_SCENE_API NNHierarchySystem final : public ISceneSystem
	{
	public:
		[[nodiscard]] NNSceneTickGroup TickGroup() const noexcept override
		{
			return NNSceneTickGroup::EarlyUpdate;
		}

		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept override;

		/**
		 * @brief 设置父节点；parent 为 **NNEntityInvalid** 表示脱离父级成为根下节点。
		 * @return 成功 true；循环引用、无效实体或 child==parent 时 false。
		 */
		bool SetParent(NNRuntimeScene& scene, NNEntity child, NNEntity parent) noexcept;

		[[nodiscard]] NNEntity GetParent(const NNRuntimeScene& scene, NNEntity entity) const noexcept;

		[[nodiscard]] std::vector<NNEntity> GetChildren(const NNRuntimeScene& scene, NNEntity entity) const;

		void OnEntityDestroyed(NNEntity entity) noexcept;

	private:
		bool WouldCreateCycle(NNEntity child, NNEntity parent) const noexcept;
		void RemoveFromParentList(NNEntity child) noexcept;
		void SyncRelationshipComponent(NNRuntimeScene& scene, NNEntity entity) noexcept;
		void RecomputeDepth(NNRuntimeScene& scene, NNEntity root, std::uint32_t depth) noexcept;

		std::unordered_map<NNEntity, NNEntity> m_ChildToParent{};
		std::unordered_map<NNEntity, std::vector<NNEntity>> m_ParentToChildren{};
	};
} // namespace NN::Runtime::Scene
