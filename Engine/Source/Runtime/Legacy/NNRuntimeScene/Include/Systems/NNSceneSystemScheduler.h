#pragma once

/**
 * @file NNSceneSystemScheduler.h
 * @brief 场景内 System 注册与按 **NNSceneTickGroup** 顺序 Tick。
 */

#include <vector>

#include "../Systems/ISceneSystem.h"
#include "../Systems/NNSceneTickGroup.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/**
	 * @brief 管理 **ISceneSystem** 列表并按帧分组调用 **Tick**。
	 */
	class NN_RUNTIME_SCENE_API NNSceneSystemScheduler
	{
	public:
		/** @brief 注册 System；同一指针重复注册将被忽略。 */
		void Register(ISceneSystem* system) noexcept;

		/** @brief 移除已注册 System；未找到则 false。 */
		bool Unregister(ISceneSystem* system) noexcept;

		/** @brief 按 EarlyUpdate → FixedUpdate → Update → LateUpdate → Render 顺序 Tick 所有已注册 System。 */
		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept;

		[[nodiscard]] std::size_t GetRegisteredCount() const noexcept;

	private:
		std::vector<ISceneSystem*> m_Systems[NNSceneTickGroupCount]{};
	};
} // namespace NN::Runtime::Scene
