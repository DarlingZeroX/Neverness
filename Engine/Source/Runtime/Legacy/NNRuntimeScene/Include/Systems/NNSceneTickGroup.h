#pragma once

/**
 * @file NNSceneTickGroup.h
 * @brief 场景 System 帧分组（与 NN::Runtime::engine::RuntimeTickGroup 数值对齐，避免 Scene 模块依赖 Engine）。
 *
 * 顺序：EarlyUpdate → FixedUpdate → Update → LateUpdate → Render。
 * Engine 侧 **NNRuntimeSceneTickSubsystem** 在 **Update** 组调用 **NNRuntimeScene::TickSystems**。
 */

#include <cstddef>
#include <cstdint>

namespace NN::Runtime::Scene
{
	enum class NNSceneTickGroup : std::uint8_t
	{
		EarlyUpdate = 0,
		FixedUpdate = 1,
		Update = 2,
		LateUpdate = 3,
		Render = 4,
	};

	inline constexpr std::size_t NNSceneTickGroupCount = 5u;
} // namespace NN::Runtime::Scene
