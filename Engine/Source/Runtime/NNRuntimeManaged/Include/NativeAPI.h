#pragma once

#include <cstdint>

#include "NNRuntimeManagedConfig.h"
#include "EngineAPIRegistry.h"

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
 * - 字符串约定：`LogInfo` 的 `messageUtf8` 为 **以 NUL 结尾的 UTF-8** 指针；禁止传 nullptr（实现端对 nullptr 做 no-op 防御）。
 * - Windows 上函数指针调用约定与托管侧 `[UnmanagedCallersOnly(CallConvStdcall)]` 对齐（x64 上 stdcall/cdecl 通常等价，仍显式标注）。
 */

#if defined(_WIN32)
#define NN_NATIVE_STDCALL __stdcall
#else
#define NN_NATIVE_STDCALL
#endif

/** 当前发布的 NNNativeAPI 布局版本（与托管 Neverness.Managed.Core 中常量保持一致）。 */
#define NN_NATIVE_API_VERSION 2u

typedef void(NN_NATIVE_STDCALL* NNNativeLogInfoFn)(const char* messageUtf8);

/**
 * @struct NNNativeAPI
 * @brief 引擎向托管代码暴露的 Native API 表（函数指针表），Phase 2 最小字段集。
 *
 * 扩展规则：仅在表尾 **追加** 新指针字段并递增 `apiVersion`；勿重排已有字段。
 */
typedef struct NativeAPI
{
	std::uint32_t apiVersion;
	/** 预留：对齐与未来 bit flags；当前填 0。 */
	std::uint32_t reserved0;
	NNNativeLogInfoFn logInfo;
	/**
	 * @brief Phase 3：指向聚合 **Engine Service ABI**（`NNNativeEngineAPI`）。
	 * @note 可為 nullptr 表示未掛載引擎服務；預設表由 `NNNativeApiTable_BuildDefault` 填有效指標。
	 */
	const struct NNNativeEngineAPI* engineServices;
} NNNativeAPI;

/** Native 侧 `logInfo` 的默认实现：写入 stderr（UTF-8 字节流）并递增内部诊断计数。 */
NN_RUNTIME_MANAGED_API void NN_NATIVE_STDCALL NNNativeApi_DefaultLogInfo(const char* messageUtf8);

/**
 * @brief 返回自进程启动以来 `NNNativeApi_DefaultLogInfo` 被调用次数（含托管经函数表间接调用）。
 * @note 供 GTest 等验证 ABI 链路；**非**游戏发行契约的一部分，后续可移入诊断通道。
 */
NN_RUNTIME_MANAGED_API std::uint32_t NNNativeApi_GetLogInfoCallCount(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
