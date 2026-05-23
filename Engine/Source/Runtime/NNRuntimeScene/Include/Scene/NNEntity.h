#pragma once

/**
 * @file NNEntity.h
 * @brief 对外暴露的实体控制代码（Handle）：仅 `uint64`，禁止向 C#/Lua 暴露 C++ 对象或 `entt::entity`。
 *
 * 打包规则与 VisionGal.Managed.Entity.EntityHandle 对齐：
 * - 低 32 位：Index（槽位，自 1 起分配；0 保留为无效）
 * - 高 32 位：Generation（同 Index 每经历一次 Destroy 递增）
 * - Index==0 且 Generation==0 => 无效（NNEntityInvalid）
 *
 * 边界：与 Legacy VGActorID、SceneSubsystem 之 NNEntityHandle、托管 EntityWorld 均无自动映射。
 */

#include <cstdint>

#include "NNNativeEngineAPI/Include/EngineHandles.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief 不透明实体句柄——复用全局 NNEntityHandle 定义，两者类型完全一致。 */
	using NNEntity = NNEntityHandle;

	/** @brief 全局无效句柄（Index=0, Generation=0）。 */
	inline constexpr NNEntity NNEntityInvalid = 0u;

	/**
	 * @brief 解包后的句柄分量（与托管 EntityHandle 字段一一对应）。
	 */
	struct NN_RUNTIME_SCENE_API NNEntityHandleParts
	{
		std::uint32_t Index = 0;
		std::uint32_t Generation = 0;
	};

	/** @brief 将 Index / Generation 打包为对外 NNEntity。 */
	[[nodiscard]] NN_RUNTIME_SCENE_API NNEntity PackEntityHandle(
		std::uint32_t index,
		std::uint32_t generation) noexcept;

	/** @brief 将 NNEntity 解包为 Index / Generation。 */
	[[nodiscard]] NN_RUNTIME_SCENE_API NNEntityHandleParts UnpackEntityHandle(NNEntity handle) noexcept;

	/** @brief 当 Index 非 0 时语义上「可参与查表」；是否仍存活须以 NNRuntimeScene::IsAlive 为准。 */
	[[nodiscard]] inline bool IsEntityHandleValid(NNEntity handle) noexcept
	{
		return UnpackEntityHandle(handle).Index != 0u;
	}
} // namespace NN::Runtime::Scene
