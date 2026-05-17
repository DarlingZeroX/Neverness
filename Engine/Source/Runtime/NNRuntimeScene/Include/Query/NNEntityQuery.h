#pragma once

/**
 * @file NNEntityQuery.h
 * @brief 对 entt::view 的薄封装：迭代时向上层暴露 NNEntity，而非 entt::entity。
 */

#include "Scene/NNEntity.h"
#include "Scene/NNRuntimeScene.h"
#include "Scene/NNWorld.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 多组件查询视图（Phase 1：同步遍历；并行 Job 留 Phase 2）。
	 * @tparam Components 须为已在 registry 上 emplace 的 POD 组件类型。
	 */
	template <typename... Components>
	class NNEntityQuery
	{
	public:
		explicit NNEntityQuery(NNRuntimeScene& scene) noexcept
			: m_Scene(&scene)
			, m_View(scene.GetRegistry().view<Components...>())
		{
		}

		/**
		 * @brief 对每个匹配实体调用回调：参数为 (NNEntity, Components&...)。
		 * @note 回调内不得 Destroy 当前实体（Phase 1 未定义重入语义）；需延迟销毁。
		 */
		template <typename Func>
		void Each(Func&& func) const
		{
			for (const auto enttEntity : m_View)
			{
				const NNEntity handle = m_Scene->HandleFromEntt(enttEntity);
				if (!m_Scene->IsAlive(handle))
				{
					continue;
				}
				std::forward<Func>(func)(handle, m_View.get<Components>(enttEntity)...);
			}
		}

		[[nodiscard]] std::size_t Size() const noexcept { return m_View.size(); }

	private:
		NNRuntimeScene* m_Scene = nullptr;
		decltype(std::declval<NNWorld>().view<Components...>()) m_View;
	};
} // namespace NN::Runtime::Scene
