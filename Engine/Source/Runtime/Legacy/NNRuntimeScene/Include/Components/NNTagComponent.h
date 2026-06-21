#pragma once

/**
 * @file NNTagComponent.h
 * @brief 轻量标签组件（纯 POD）：位标志 + 实体名称。
 */

#include <cstdint>
#include <cstring>

#include "../..//NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief 标签组件：Flags 位掩码 + 固定大小 Name 缓冲区（POD 兼容）。 */
	struct NN_RUNTIME_SCENE_API NNTagComponent
	{
		std::uint32_t Flags = 0u;
		char Name[64] = {};  // 实体显示名（UTF-8，零结尾）
	};
} // namespace NN::Runtime::Scene
