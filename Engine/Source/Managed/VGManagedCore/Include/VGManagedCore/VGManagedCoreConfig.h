#pragma once

/**
 * @file VGManagedCoreConfig.h
 * @brief VGManagedCore 编译配置宏。
 *
 * Phase 2 中本模块以 **静态库** 形式链接进 VGManagedHost DLL，
 * 一般无需 dllexport。若未来改为独立 DLL，可在此扩展 VG_MANAGED_CORE_API。
 */

#if defined(_WIN32)
#if defined(VG_MANAGED_CORE_BUILD_SHARED) && defined(VG_MANAGED_CORE_EXPORT)
#define VG_MANAGED_CORE_API __declspec(dllexport)
#elif defined(VG_MANAGED_CORE_BUILD_SHARED)
#define VG_MANAGED_CORE_API __declspec(dllimport)
#else
#define VG_MANAGED_CORE_API
#endif
#else
#if defined(VG_MANAGED_CORE_EXPORT)
#define VG_MANAGED_CORE_API __attribute__((visibility("default")))
#else
#define VG_MANAGED_CORE_API
#endif
#endif
