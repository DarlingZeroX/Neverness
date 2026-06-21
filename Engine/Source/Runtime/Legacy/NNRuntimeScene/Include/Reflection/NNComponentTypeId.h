#pragma once

/**
 * @file NNComponentTypeId.h
 * @brief 组件类型稳定整数标识（FNV-1a name hash，跨进程/跨版本稳定）。
 */

#include <cstdint>

namespace NN::Runtime::Scene
{
		/** @brief 组件类型标识 = FNV-1a 64-bit(name)，跨平台稳定。 */
		using NNComponentTypeId = std::uint64_t;

		/** @brief 无效组件类型 id。 */
		inline constexpr NNComponentTypeId NNComponentTypeIdInvalid = 0u;
} // namespace NN::Runtime::Scene
