#pragma once

/**
 * @file RmlUITypes.h
 * @brief RmlUI 本地类型定义——替代 NNRuntimeScene 中的类型。
 *
 * 原 NNEntity / NNRmlUIViewTarget / NNRmlUIDocumentFlags / IAssetResolver
 * 已随 NNRuntimeScene 移至 Legacy。此文件定义 RmlUI 模块自用的等价类型。
 */

#include <cstdint>
#include "Engine/EngineTypes.h"

namespace NN::Runtime::RmlUI
{
	/// @brief 实体 ID（原 NNEntity，简化为 uint64_t）。
	using NNEntity = std::uint64_t;

	/// @brief 实体哈希（用于 unordered_map）。
	struct NNEntityHash
	{
		std::size_t operator()(NNEntity e) const noexcept
		{
			return std::hash<std::uint64_t>{}(e);
		}
	};

	/// @brief RmlUI 视图目标枚举（原 NNRmlUIViewTarget）。
	enum class NNRmlUIViewTarget : std::uint32_t
	{
		Scene = 0,   ///< 场景视口
		Game  = 1,   ///< 游戏视口
		Both  = 2,   ///< 两者都渲染
	};

} // namespace NN::Runtime::RmlUI
