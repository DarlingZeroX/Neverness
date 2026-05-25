#pragma once

/**
 * @file EventApiExport.h
 * @brief 填充指向进程内 **EventQueue** + **SDL3EventTranslator** 的 **NNEventAPI**。
 */

#include "NNNativeEngineAPI/Include/EventAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 将 Runtime Event 子表写入 **NNNativeEngineAPI::events**。 */
void NNBuildEventRuntimeApi(NNEventAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
