#pragma once

#include "EngineServicesConfig.h"
#include "NativeAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ManagedExports.h
 * @brief 向托管侧或宿主进程暴露 **只读** 默认 API 表指针（单例）。
 *
 * 典型用法：Native 在 `load_assembly_and_get_function_pointer` 解析到托管 Bootstrap 后，
 * 将 `NNNativeApi_GetDefaultTable()` 的返回值作为参数传入 `[UnmanagedCallersOnly]` 方法。
 *
 * 生命周期：指针在首次调用前由实现线程安全初始化；进程内地址稳定至进程结束（Phase 2 静态存储期）。
 */

NN_API const NNNativeAPI* NNNativeApi_GetDefaultTable(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
