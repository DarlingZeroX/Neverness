#pragma once

/**
 * @file NNComponentFieldType.h
 * @brief 组件字段反射类型（序列化 / C# 绑定 / Inspector 预留）。
 */

#include <cstdint>

namespace NN::Runtime::Scene
{
		/** @brief 组件字段数据类型枚举。 */
		enum class NNComponentFieldType : std::uint8_t
		{
			Float = 0,
			Float3,
			Float4,
			Float4x4,
			Quaternion,
			UInt32,
			UInt64,
			Entity,
			CharArray,  // 固定大小字符数组（按 NNComponentFieldDesc.Size 字节 memcpy）
		};
} // namespace NN::Runtime::Scene
