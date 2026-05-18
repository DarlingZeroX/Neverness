#pragma once

#include <cstdint>

#include "NNRuntimeManagedConfig.h"

/**
 * @file ManagedHandle.h
 * @brief 跨 Native/Managed 边界的 **不透明句柄**（POD），Phase 2 仅占位。
 *
 * Gameplay / Dialogue 等上层类型尚未 ABI 化；此处仅统一命名与宽度（64-bit），
 * 避免将来各模块自行 typedef 导致不一致。
 */

#ifdef __cplusplus
extern "C" {
#endif

/** 实体句柄（占位）：0 表示无效，非 0 含义由后续 Gameplay 合约定义。 */
typedef std::uint64_t NNEntityHandle;

/** 资产句柄（占位）。 */
typedef std::uint64_t NNAssetHandle;

#ifdef __cplusplus
} /* extern "C" */
#endif
