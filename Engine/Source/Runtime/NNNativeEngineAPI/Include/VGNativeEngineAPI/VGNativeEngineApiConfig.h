#pragma once

/**
 * @file VGNativeEngineApiConfig.h
 * @brief VGNativeEngineAPI 模組匯出宏（靜態庫預設為空）。
 */

#if defined(_WIN32)
#if defined(VG_NATIVE_ENGINE_API_BUILD_SHARED) && defined(VG_NATIVE_ENGINE_API_EXPORT)
#define VG_NATIVE_ENGINE_API __declspec(dllexport)
#elif defined(VG_NATIVE_ENGINE_API_BUILD_SHARED)
#define VG_NATIVE_ENGINE_API __declspec(dllimport)
#else
#define VG_NATIVE_ENGINE_API
#endif
#else
#if defined(VG_NATIVE_ENGINE_API_EXPORT)
#define VG_NATIVE_ENGINE_API __attribute__((visibility("default")))
#else
#define VG_NATIVE_ENGINE_API
#endif
#endif
