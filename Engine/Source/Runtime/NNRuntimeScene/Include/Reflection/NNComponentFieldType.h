#pragma once

/**
 * @file NNComponentFieldType.h
 * @brief 组件字段反射类型（序列化 / C# 绑定 / Inspector 预留）。
 */

#include <cstdint>

namespace NN::Runtime::Scene
{
	enum class NNComponentFieldType : std::uint8_t
	{
		Float = 0,
		Float3,
		UInt32,
		UInt64,
		Entity,
	};
} // namespace NN::Runtime::Scene
