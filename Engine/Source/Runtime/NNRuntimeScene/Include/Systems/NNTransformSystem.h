#pragma once

/**
 * @file NNTransformSystem.h
 * @brief 变换 System：遍历 **NNTransformComponent**（Phase 2 为本地空间占位，world 矩阵留 Phase 3+）。
 */

#include "Systems/ISceneSystem.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief **Update** 组内对 Transform 做批量访问；当前不写回派生数据，仅供渲染链与测试挂钩。
	 */
	class NN_RUNTIME_SCENE_API NNTransformSystem final : public ISceneSystem
	{
	public:
		[[nodiscard]] NNSceneTickGroup TickGroup() const noexcept override
		{
			return NNSceneTickGroup::Update;
		}

		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept override;

		[[nodiscard]] std::uint32_t GetLastTickedTransformCount() const noexcept
		{
			return m_LastTickedCount;
		}

	private:
		std::uint32_t m_LastTickedCount = 0u;
	};
} // namespace NN::Runtime::Scene
