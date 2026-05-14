#pragma once

#include <cstdint>

#include "VGManagedCore/VGManagedCoreConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file NativeAPI.h
 * @brief VisionGal **Native → Managed** 导出函数表（VGNativeAPI）的 C ABI 头。
 *
 * 设计要点：
 * - 仅使用 C 可互操作类型，便于 C# `LayoutKind.Sequential` 逐字段对齐镜像。
 * - 首字段 `apiVersion`：破坏性变更时递增；托管侧应先校验再解引用函数指针。
 * - 字符串约定：`LogInfo` 的 `messageUtf8` 为 **以 NUL 结尾的 UTF-8** 指针；禁止传 nullptr（实现端对 nullptr 做 no-op 防御）。
 * - Windows 上函数指针调用约定与托管侧 `[UnmanagedCallersOnly(CallConvStdcall)]` 对齐（x64 上 stdcall/cdecl 通常等价，仍显式标注）。
 */

#if defined(_WIN32)
#define VG_NATIVE_STDCALL __stdcall
#else
#define VG_NATIVE_STDCALL
#endif

/** 当前发布的 VGNativeAPI 布局版本（与托管 VisionGal.Managed.Core 中常量保持一致）。 */
#define VG_NATIVE_API_VERSION 2u

struct VGNativeEngineAPI;

typedef void(VG_NATIVE_STDCALL* VGNativeLogInfoFn)(const char* messageUtf8);

/**
 * @struct VGNativeAPI
 * @brief 引擎向托管代码暴露的 Native API 表（函数指针表），Phase 2 最小字段集。
 *
 * 扩展规则：仅在表尾 **追加** 新指针字段并递增 `apiVersion`；勿重排已有字段。
 */
typedef struct VGNativeAPI
{
	std::uint32_t apiVersion;
	/** 预留：对齐与未来 bit flags；当前填 0。 */
	std::uint32_t reserved0;
	VGNativeLogInfoFn logInfo;
	/**
	 * @brief Phase 3：指向聚合 **Engine Service ABI**（`VGNativeEngineAPI`）。
	 * @note 可為 nullptr 表示未掛載引擎服務；預設表由 `VGNativeApiTable_BuildDefault` 填有效指標。
	 */
	const struct VGNativeEngineAPI* engineServices;
} VGNativeAPI;

/** Native 侧 `logInfo` 的默认实现：写入 stderr（UTF-8 字节流）并递增内部诊断计数。 */
VG_MANAGED_CORE_API void VG_NATIVE_STDCALL VGNativeApi_DefaultLogInfo(const char* messageUtf8);

/**
 * @brief 返回自进程启动以来 `VGNativeApi_DefaultLogInfo` 被调用次数（含托管经函数表间接调用）。
 * @note 供 GTest 等验证 ABI 链路；**非**游戏发行契约的一部分，后续可移入诊断通道。
 */
VG_MANAGED_CORE_API std::uint32_t VGNativeApi_GetLogInfoCallCount(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
