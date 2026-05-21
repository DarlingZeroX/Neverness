#pragma once

#include "NNNativeEngineAPI/Include/ApplicationAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 填充指向进程内 **RuntimeApplication** 单例的 **NNApplicationAPI**。 */
NNApplicationAPI BuildApplicationApi(void);

/** @brief 将 Runtime Application 子表写入 **NNNativeEngineAPI::application**。 */
void NNBuildApplicationRuntimeApi(NNApplicationAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
