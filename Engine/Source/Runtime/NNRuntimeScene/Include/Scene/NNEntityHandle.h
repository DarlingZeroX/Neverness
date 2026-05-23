#pragma once

/**
 * @file NNEntityHandle.h
 * @brief 实体槽位与世代表（内部实现细节）；对外仅通过 NNEntity (uint64) 交互。
 */

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <NNCore/Include/Scene/entt.hpp>

#include "../Scene/NNEntity.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 单槽位内部状态（与托管 EntityWorld.EntityRecord 语义对齐）。
	 */
	struct NN_RUNTIME_SCENE_API EntitySlot
	{
		std::uint32_t Generation = 0u;
		bool Alive = false;
		entt::entity EnttEntity = entt::null;
	};

	/**
	 * @brief 实体 Index / Generation 分配与 entt 实体映射（Phase 1 单线程）。
	 */
	class NN_RUNTIME_SCENE_API NNEntityHandleTable
	{
	public:
		/**
		 * @brief 分配新实体槽位并在 registry 中 create。
		 * @param outEntt 输出 entt 实体（供 Emplace 使用）。
		 * @return 打包后的 NNEntity；失败时返回 NNEntityInvalid。
		 */
		[[nodiscard]] NNEntity Create(entt::registry& registry, entt::entity& outEntt) noexcept;

		/**
		 * @brief 销毁实体：校验世代、递增 generation、destroy entt 实体。
		 * @return 世代匹配且曾存活时 true。
		 */
		bool Destroy(entt::registry& registry, NNEntity handle) noexcept;

		[[nodiscard]] bool IsAlive(NNEntity handle) const noexcept;

		/**
		 * @brief 将对外句柄解析为 entt::entity；无效或世代不符时返回 entt::null。
		 * @note 仅供 C++ System / NNRuntimeScene 内部使用，不得暴露给 ABI。
		 */
		[[nodiscard]] entt::entity Resolve(NNEntity handle) const noexcept;

		/** @brief 由 entt 实体反查对外句柄（用于 Query 迭代）。 */
		[[nodiscard]] NNEntity HandleFromEntt(entt::entity entity) const noexcept;

		[[nodiscard]] std::size_t GetSlotCount() const noexcept { return m_Slots.size(); }

		/** @brief 遍历当前存活实体（供序列化等工具使用）。 */
		template <typename Func>
		void ForEachAlive(Func&& func) const
		{
			for (std::uint32_t index = 1u; index < m_Slots.size(); ++index)
			{
				const EntitySlot& slot = m_Slots[index];
				if (!slot.Alive)
				{
					continue;
				}
				const NNEntity handle = PackEntityHandle(index, slot.Generation);
				std::forward<Func>(func)(handle);
			}
		}

	private:
		void EnsureSlotCapacity(std::uint32_t index);

		// Index 0 保留；有效 Index 自 1 起（与托管 EntityWorld 一致）。
		std::vector<EntitySlot> m_Slots{1};
		std::uint32_t m_NextIndex = 1u;
		/** @brief entt 实体 → 对外 Index（键为 entt::to_integral，避免依赖 entity::hasher）。 */
		std::unordered_map<std::uint32_t, std::uint32_t> m_EnttToIndex{};
	};
} // namespace NN::Runtime::Scene
