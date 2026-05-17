#pragma once

/**
 * @file NNRuntimeSceneTickSubsystem.h
 * @brief 将 **NNRuntimeScene::TickSystems** 挂接到 **RuntimeScheduler** 的 **Update** 组。
 *
 * 依赖方向：**NNRuntimeEngine** 链接 **NevernessRuntime-Scene**；Scene 模块不依赖 Engine。
 */

#include <cstdint>

#include "RuntimeScheduler/RuntimeSubsystem.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;
}

namespace NN::Runtime::engine
{
	/**
	 * @brief 每帧在 **Update** 阶段驱动 ECS 场景 System 管线。
	 */
	class NNRuntimeSceneTickSubsystem final : public IRuntimeSubsystem
	{
	public:
		explicit NNRuntimeSceneTickSubsystem(Scene::NNRuntimeScene* scene = nullptr) noexcept;

		void SetScene(Scene::NNRuntimeScene* scene) noexcept;

		void Initialize() noexcept override {}
		void Shutdown() noexcept override {}

		void Tick(const RuntimeFrameContext& context) noexcept override;

		[[nodiscard]] RuntimeTickGroup TickGroup() const noexcept override;

		[[nodiscard]] std::uint32_t GetTickInvocationCount() const noexcept
		{
			return tickInvocationCount_;
		}

	private:
		Scene::NNRuntimeScene* scene_ = nullptr;
		std::uint32_t tickInvocationCount_ = 0u;
	};
} // namespace NN::Runtime::engine
