#pragma once

/**
 * @file RuntimeRmlUIExport.h
 * @brief NNRuntimeRmlui 模块导出宏。
 *
 * 与 VGUIConfig.h 共用 VG_UI_EXPORT 定义。
 * ABI 目录下的 extern "C" 函数通过此宏导出。
 */

#ifdef VG_UI_EXPORT
#if defined(_WIN32) || defined(_WIN64)
#define NN_RUNTIME_RMLUI_API __declspec(dllexport)
#else
#define NN_RUNTIME_RMLUI_API __attribute__((visibility("default")))
#endif
#else
#if defined(_WIN32) || defined(_WIN64)
#define NN_RUNTIME_RMLUI_API __declspec(dllimport)
#else
#define NN_RUNTIME_RMLUI_API
#endif
#endif
