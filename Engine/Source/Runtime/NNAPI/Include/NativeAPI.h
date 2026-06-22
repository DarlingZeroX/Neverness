#pragma once

#include <cstdint>

#include "NNAPIConfig.h"
#include "Engine/EngineAPIRegistry.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file NativeAPI.h
 * @brief VisionGal **Native → Managed** 导出函数表（NNNativeAPI）的 C ABI 头。
 *
 * 设计要点：
 * - 仅使用 C 可互操作类型，便于 C# `LayoutKind.Sequential` 逐字段对齐镜像。
 * - 首字段 `apiVersion`：破坏性变更时递增；托管侧应先校验再解引用函数指针。
 */

/** 当前发布的 NNNativeAPI 布局版本（与托管 Neverness.Managed.Core 中常量保持一致）。 */
#define NN_NATIVE_API_VERSION 3u

/**
 * @struct NNNativeAPI
 * @brief 引擎向托管代码暴露的 Native API 表（函数指针表）。
 *
 * 扩展规则：仅在表尾 **追加** 新指针字段并递增 `apiVersion`；勿重排已有字段。
 */
typedef struct NativeAPI
{
	std::uint32_t apiVersion;
	/** 预留：对齐与未来 bit flags；当前填 0。 */
	std::uint32_t reserved0;
	/**
	 * @brief 指向聚合 **Engine Service ABI**（`NNNativeEngineAPI`）。
	 * @note 可為 nullptr 表示未掛載引擎服務；預設表由 `NNNativeApiTable_BuildDefault` 填有效指標。
	 */
	const struct NNNativeEngineAPI* engineServices;
} NNNativeAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
