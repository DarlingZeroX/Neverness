#pragma once

/**
 * @file NNTransformSystem.h
 * @brief 变换 System：DFS 遍历层级树，计算每个实体的 LocalMatrix 与 WorldMatrix。
 *
 * TickGroup 为 Update；对根节点（无父）先算局部矩阵，
 * 再 DFS 向下逐级 WorldMatrix = Parent.WorldMatrix * LocalMatrix。
 */

#include "../Systems/ISceneSystem.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief Update 组内对 Transform 做批量访问与世界矩阵计算。
	 *
	 * 对每个有 TransformComponent 的实体：
	 *   - 无 Relationship 或 Parent==Invalid → 根节点，LocalMatrix 即 WorldMatrix
	 *   - 有父节点 → DFS 递归，WorldMatrix = Parent.WorldMatrix * LocalMatrix
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
