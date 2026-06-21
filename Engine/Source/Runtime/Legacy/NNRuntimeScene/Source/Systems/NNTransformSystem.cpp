/**
 * @file NNTransformSystem.cpp
 * @brief NNTransformSystem 实现：DFS 遍历层级，计算每个实体的世界变换矩阵。
 *
 * 算法概要：
 *   1. 先遍历所有根节点（无 Relationship 或 Parent==Invalid），计算 LocalMatrix
 *   2. 对根节点的子树做 DFS，子节点 WorldMatrix = Parent.WorldMatrix * LocalMatrix
 */

#include "Systems/NNTransformSystem.h"

#include <NNCore/Include/Math/GLM/ext/matrix_transform.hpp>
#include <NNCore/Include/Math/GLM/gtc/quaternion.hpp>

#include "Components/NNRelationshipComponent.h"
#include "Components/NNTransformComponent.h"
#include "Scene/NNRuntimeScene.h"
#include "Systems/NNHierarchySystem.h"

namespace NN::Runtime::Scene
{
namespace
{
/// 从 NNTransformComponent 的 Position/Rotation/Scale 构建局部变换矩阵（TRS）
glm::mat4 BuildLocalMatrix(const NNTransformComponent& transform)
{
	// 平移矩阵
	const glm::mat4 T = glm::translate(glm::identity<glm::mat4>(), transform.Position);

	// 旋转矩阵（四元数 → 旋转矩阵）
	const glm::mat4 R = glm::mat4_cast(transform.Rotation);

	// 缩放矩阵
	const glm::mat4 S = glm::scale(glm::identity<glm::mat4>(), transform.Scale);

	// TRS 组合：先缩放，再旋转，最后平移
	return T * R * S;
}

/// 递归 DFS：计算子树中每个实体的世界矩阵
void DFSComputeWorldMatrix(
	NNRuntimeScene& scene,
	const NNEntity entity,
	const glm::mat4& parentWorld)
{
	NNTransformComponent* transform = scene.TryGet<NNTransformComponent>(entity);
	if (transform == nullptr)
	{
		return;
	}

	// 世界矩阵 = 父世界矩阵 * 局部矩阵
	transform->WorldMatrix = parentWorld * BuildLocalMatrix(*transform);

	// 递归处理子节点
	const auto children = scene.GetChildren(entity);
	for (const NNEntity child : children)
	{
		DFSComputeWorldMatrix(scene, child, transform->WorldMatrix);
	}
}
} // namespace

void NNTransformSystem::Tick(NNRuntimeScene& scene, const float deltaTimeSeconds) noexcept
{
	(void)deltaTimeSeconds;
	m_LastTickedCount = 0u;

	// 遍历所有拥有 TransformComponent 的实体，仅处理根节点
	scene.Query<NNTransformComponent>().Each(
		[this, &scene](NNEntity handle, NNTransformComponent& /*transform*/)
		{
			// 通过 handle 获取组件指针，避免 view 引用可能的 ABI 问题
			NNTransformComponent* transformPtr = scene.TryGet<NNTransformComponent>(handle);
			if (transformPtr == nullptr)
			{
				return;
			}

			// 仅处理根节点：没有 Relationship 组件，或 Parent 为无效
			if (scene.Has<NNRelationshipComponent>(handle))
			{
				const NNRelationshipComponent* rel = scene.TryGet<NNRelationshipComponent>(handle);
				if (rel != nullptr && rel->Parent != NNEntityInvalid)
				{
					return; // 非根节点，跳过（由 DFS 递归处理）
				}
			}

			// 根节点的局部矩阵即世界矩阵（父矩阵为单位阵）
			transformPtr->WorldMatrix = BuildLocalMatrix(*transformPtr);
			++m_LastTickedCount;

			// DFS 处理子树
			const auto children = scene.GetChildren(handle);
			for (const NNEntity child : children)
			{
				DFSComputeWorldMatrix(scene, child, transformPtr->WorldMatrix);
			}
		});
}
} // namespace NN::Runtime::Scene
