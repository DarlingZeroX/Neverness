#pragma once

/**
 * @file NNRuntimeManagedHostConfig.h
 * @brief NNRuntimeManagedHost 编译配置宏（CoreCLR 宿主 SHARED 库）。
 */

#if defined(_WIN32)
#if defined(NN_RUNTIME_MANAGED_HOST_BUILD_SHARED) && defined(NN_RUNTIME_MANAGED_HOST_EXPORT)
#define NN_RUNTIME_MANAGED_HOST_API __declspec(dllexport)
#elif defined(NN_RUNTIME_MANAGED_HOST_BUILD_SHARED)
#define NN_RUNTIME_MANAGED_HOST_API __declspec(dllimport)
#else
#define NN_RUNTIME_MANAGED_HOST_API
#endif
#else
#if defined(NN_RUNTIME_MANAGED_HOST_EXPORT)
#define NN_RUNTIME_MANAGED_HOST_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_MANAGED_HOST_API
#endif
#endif

/** @deprecated 兼容旧宏名；新代码请使用 NN_RUNTIME_MANAGED_HOST_API。 */
#define VG_MANAGED_HOST_API NN_RUNTIME_MANAGED_HOST_API
