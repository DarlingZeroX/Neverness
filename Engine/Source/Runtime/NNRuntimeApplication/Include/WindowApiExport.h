#pragma once

#include "NNNativeEngineAPI/Include/WindowAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 填充指向进程内 **WindowRegistry** 的 **NNWindowAPI**。 */
void NNBuildWindowRuntimeApi(NNWindowAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
