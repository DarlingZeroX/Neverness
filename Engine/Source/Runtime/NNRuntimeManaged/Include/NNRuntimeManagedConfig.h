#pragma once

/**
 * @file NNRuntimeManagedConfig.h
 * @brief NNRuntimeManaged 编译配置宏。
 *
 * 本模块以 **SHARED** 库导出 Native ↔ Managed ABI（`NNNativeAPI` 函数表）。
 */

#if defined(_WIN32)
#if defined(NN_RUNTIME_MANAGED_BUILD_SHARED) && defined(NN_RUNTIME_MANAGED_EXPORT)
#define NN_RUNTIME_MANAGED_API __declspec(dllexport)
#elif defined(NN_RUNTIME_MANAGED_BUILD_SHARED)
#define NN_RUNTIME_MANAGED_API __declspec(dllimport)
#else
#define NN_RUNTIME_MANAGED_API
#endif
#else
#if defined(NN_RUNTIME_MANAGED_EXPORT)
#define NN_RUNTIME_MANAGED_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_MANAGED_API
#endif
#endif
