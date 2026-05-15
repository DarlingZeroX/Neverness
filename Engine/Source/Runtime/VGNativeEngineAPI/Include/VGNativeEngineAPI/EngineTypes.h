#pragma once

/**
 * @file EngineTypes.h
 * @brief 跨 ABI 邊界之 POD 結構（Phase 5 layout v3）。
 */

#include <cstdint>

#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 128-bit 資產 GUID（與託管 VisionGal.Managed.Assets.GUID 對齊）。 */
typedef struct VGGuid
{
	std::uint64_t high;
	std::uint64_t low;
} VGGuid;

/** @brief 三維變換（位置 / 歐拉角 / 縮放，單位由實作定義）。 */
typedef struct VGTransform3
{
	float position[3];
	float rotation[3];
	float scale[3];
} VGTransform3;

#ifdef __cplusplus
} /* extern "C" */
#endif
