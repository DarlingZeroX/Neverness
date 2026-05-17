#pragma once

/**
 * @file NNTagComponent.h
 * @brief 轻量标签位掩码组件（纯 POD），用于查询过滤与编辑器标记。
 */

#include <cstdint>

#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief 位标志标签；具体语义由游戏/工具层约定。 */
	struct NN_RUNTIME_SCENE_API NNTagComponent
	{
		std::uint32_t Flags = 0u;
	};
} // namespace NN::Runtime::Scene
