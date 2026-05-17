#pragma once

/**
 * @file ISceneSystem.h
 * @brief 场景 System 接口：行为逻辑由 System 驱动，组件保持 POD。
 */

#include "Systems/NNSceneTickGroup.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/**
	 * @brief 可在 **NNSceneSystemScheduler** 中注册的运行时 System。
	 * @note Phase 2 单线程；**Tick** 内避免对当前正在迭代的实体调用 **DestroyEntity**（未定义重入）。
	 */
	class ISceneSystem
	{
	public:
		virtual ~ISceneSystem() = default;

		[[nodiscard]] virtual NNSceneTickGroup TickGroup() const noexcept = 0;

		virtual void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept = 0;
	};
} // namespace NN::Runtime::Scene
