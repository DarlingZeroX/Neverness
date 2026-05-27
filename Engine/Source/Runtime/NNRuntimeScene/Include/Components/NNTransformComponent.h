#pragma once

/**
 * @file NNTransformComponent.h
 * @brief 变换组件（POD，无虚表）：使用引擎核心类型 float3 / quaternion / matrix。
 *
 * 与 Legacy VGEngineRuntime::TransformComponent 字段语义对齐，
 * 但保持纯数据 POD 布局，行为由 NNTransformSystem 驱动。
 */

#include <NNCore/Interface/HVector.h>
#include "../..//NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 实体局部变换 + 世界矩阵。
	 *
	 * Position / Rotation / Scale 为局部空间；Rotation 使用四元数（归一化）。
	 * WorldMatrix 由 NNTransformSystem 在 Tick 中通过 DFS 层级遍历计算。
	 */
	struct NN_RUNTIME_SCENE_API NNTransformComponent
	{
		Core::float3 Position;        ///< 局部平移
		Core::quaternion Rotation; ///< 局部旋转（四元数 wxyz，默认单位）
		Core::float3 Scale;           ///< 局部缩放
		Core::matrix WorldMatrix;                ///< 世界变换矩阵（由 NNTransformSystem 计算，默认单位阵）

		NNTransformComponent()
			:Position(0.f, 0.f, 0.f),
			Rotation(0.f, 0.f, 0.f, 1.f),
			Scale(1.f, 1.f, 1.f),
			WorldMatrix(1.f)
		{
		}
	};
} // namespace NN::Runtime::Scene
