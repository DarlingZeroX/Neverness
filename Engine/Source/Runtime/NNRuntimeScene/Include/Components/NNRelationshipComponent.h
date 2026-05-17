#pragma once

/**
 * @file NNRelationshipComponent.h
 * @brief 层级关系组件（纯 POD）：仅存 Handle 与计数，不持有 IEntity* 或 entt 指针图。
 */

#include "Scene/NNEntity.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 场景树关系数据（Phase 1 仅存储；NNHierarchySystem 于 Phase 2 维护一致性）。
	 */
	struct NN_RUNTIME_SCENE_API NNRelationshipComponent
	{
		/** @brief 父实体句柄；NNEntityInvalid 表示根下无父。 */
		NNEntity Parent{NNEntityInvalid};

		/** @brief 直接子节点数量（由层级 System 更新）。 */
		std::uint32_t ChildCount = 0u;

		/** @brief 自根起的深度（根为 0）。 */
		std::uint32_t Depth = 0u;
	};
} // namespace NN::Runtime::Scene
