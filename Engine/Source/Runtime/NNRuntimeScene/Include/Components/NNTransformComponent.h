#pragma once

/**
 * @file NNTransformComponent.h
 * @brief 变换组件（纯 POD）：与 NNTransform3 字段布局一致，便于未来 ABI 与序列化对齐。
 */

#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 实体局部变换（无虚表、无 Update）。
	 * @note 行为由 NNTransformSystem（Phase 2）或渲染 System 读取本数据驱动。
	 */
	struct NN_RUNTIME_SCENE_API NNTransformComponent
	{
		float Position[3]{0.f, 0.f, 0.f};
		float Rotation[3]{0.f, 0.f, 0.f};
		float Scale[3]{1.f, 1.f, 1.f};
	};
} // namespace NN::Runtime::Scene
