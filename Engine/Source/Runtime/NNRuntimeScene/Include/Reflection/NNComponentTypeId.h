#pragma once

/**
 * @file NNComponentTypeId.h
 * @brief 组件类型稳定整数标识（用于序列化、C# 绑定与编辑器反射，Phase 1 仅元数据登记）。
 */

#include <cstdint>

namespace NN::Runtime::Scene
{
	using NNComponentTypeId = std::uint32_t;

	/** @brief 无效组件类型 id。 */
	inline constexpr NNComponentTypeId NNComponentTypeIdInvalid = 0u;
} // namespace NN::Runtime::Scene
