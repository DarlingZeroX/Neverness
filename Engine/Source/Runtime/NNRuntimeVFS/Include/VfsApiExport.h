#pragma once

#include "NNNativeEngineAPI/Include/VfsAPI.h"
#include "../RuntimeVFSExport.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 填充指向 **VFSService** 的 **NNVfsAPI** 函数表。 */
	NN_RUNTIME_VFS_API void NNBuildVfsRuntimeApi(NNVfsAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
