#pragma once

/**
 * @file NNWorld.h
 * @brief ECS 世界类型别名：底层为 entt::registry（组件纯数据，行为由 System 驱动）。
 */

#include <NNCore/Include/Scene/entt.hpp>

namespace NN::Runtime::Scene
{
	/**
	 * @brief 场景组件存储后端（entt 注册表）。
	 * @note Phase 1 由 NNRuntimeScene 独占持有；不暴露给托管层直接操作。
	 */
	using NNWorld = entt::registry;
} // namespace NN::Runtime::Scene
