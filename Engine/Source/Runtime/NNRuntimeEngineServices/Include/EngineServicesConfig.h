#pragma once

/**
 * @file EngineServicesConfig.h
 * @brief NNRuntimeEngineServices 模块导出/导入宏。
 *
 * NN_API 用于标注从本 DLL 导出的 C ABI 函数（如 NNNativeApi_GetDefaultTable）。
 */

#if defined(_WIN32)
#if defined(NN_ENGINE_SERVICES_EXPORT)
#define NN_API __declspec(dllexport)
#else
#define NN_API __declspec(dllimport)
#endif
#else
#if defined(NN_ENGINE_SERVICES_EXPORT)
#define NN_API __attribute__((visibility("default")))
#else
#define NN_API
#endif
#endif
