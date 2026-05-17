#pragma once

/**
 * @file NNRuntimeSceneExport.h
 * @brief NevernessRuntime-Scene 动态库导出宏（与 NNRuntimeCore 之 NN_RUNTIME_CORE_API 模式一致）。
 */

#ifdef NN_RUNTIME_SCENE_DYNAMIC
#if defined(_WIN32) || defined(_WIN64)
#ifdef NN_RUNTIME_SCENE_EXPORT
#define NN_RUNTIME_SCENE_API __declspec(dllexport)
#else
#define NN_RUNTIME_SCENE_API __declspec(dllimport)
#endif
#else
#ifdef NN_RUNTIME_SCENE_EXPORT
#define NN_RUNTIME_SCENE_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_SCENE_API
#endif
#endif
#else
#define NN_RUNTIME_SCENE_API
#endif
