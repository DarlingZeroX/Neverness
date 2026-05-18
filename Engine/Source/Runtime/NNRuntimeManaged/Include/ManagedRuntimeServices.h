#pragma once

#include "NNRuntimeManagedConfig.h"
#include "NativeAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ManagedRuntimeServices.h
 * @brief 构建 / 注册默认 **NNNativeAPI** 表的服务入口（Phase 2：单一路径 + 预留扩展钩子）。
 */

/**
 * @brief 将 `outTable` 填充为当前进程默认的 Native API 表（函数指针指向引擎实现）。
 * @param outTable 非空；写入前会将 reserved 字段清零。
 */
NN_RUNTIME_MANAGED_API void NNNativeApiTable_BuildDefault(NNNativeAPI* outTable);

/**
 * @brief 预留：将自定义 `logInfo` 注册到进程级默认表构建流程（Phase 2 可为 no-op 或仅保存函数指针副本）。
 *
 * 当前 Phase 2 未实现插件化覆盖；若调用，行为为 **不改变** `NNNativeApiTable_BuildDefault` 结果（仅占位 API）。
 */
NN_RUNTIME_MANAGED_API void NNNativeApi_RegisterLogInfoOverride(NNNativeLogInfoFn overrideFn);

#ifdef __cplusplus
} /* extern "C" */
#endif
