#pragma once

#include "EngineServicesConfig.h"
#include "NativeAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ManagedRuntimeServices.h
 * @brief 构建默认 **NNNativeAPI** 表的服务入口。
 */

/**
 * @brief 将 `outTable` 填充为当前进程默认的 Native API 表（函数指针指向引擎实现）。
 * @param outTable 非空；写入前会将 reserved 字段清零。
 */
NN_API void NNNativeApiTable_BuildDefault(NNNativeAPI* outTable);
  
  
#ifdef __cplusplus
} /* extern "C" */
#endif
