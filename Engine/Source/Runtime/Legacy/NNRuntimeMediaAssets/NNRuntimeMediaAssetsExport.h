#pragma once

/**
 * @file NNRuntimeMediaAssetsExport.h
 * @brief NevernessRuntime-MediaAssets 动态库导出宏。
 */

#ifdef NN_RUNTIME_MEDIA_ASSETS_DYNAMIC
#if defined(_WIN32) || defined(_WIN64)
#ifdef NN_RUNTIME_MEDIA_ASSETS_EXPORT
#define NN_RUNTIME_MEDIA_ASSETS_API __declspec(dllexport)
#else
#define NN_RUNTIME_MEDIA_ASSETS_API __declspec(dllimport)
#endif
#else
#ifdef NN_RUNTIME_MEDIA_ASSETS_EXPORT
#define NN_RUNTIME_MEDIA_ASSETS_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_MEDIA_ASSETS_API
#endif
#endif
#else
#define NN_RUNTIME_MEDIA_ASSETS_API
#endif
