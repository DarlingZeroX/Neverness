#pragma once

/**
 * @file NNSceneUpdateSystem.h
 * @brief 场景帧更新 System（Phase 2 占位：可扩展全局逻辑，当前仅递增帧计数供测试）。
 */

#include "../Systems/ISceneSystem.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief **EarlyUpdate** 组；在层级 System 之后执行场景级逻辑钩子。 */
	class NN_RUNTIME_SCENE_API NNSceneUpdateSystem final : public ISceneSystem
	{
	public:
		[[nodiscard]] NNSceneTickGroup TickGroup() const noexcept override
		{
			return NNSceneTickGroup::EarlyUpdate;
		}

		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept override;

		[[nodiscard]] std::uint32_t GetFrameCounter() const noexcept { return m_FrameCounter; }

	private:
		std::uint32_t m_FrameCounter = 0u;
	};
} // namespace NN::Runtime::Scene
