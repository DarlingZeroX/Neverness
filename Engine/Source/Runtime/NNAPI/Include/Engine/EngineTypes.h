#pragma once

/**
 * @file EngineTypes.h
 * @brief 跨 ABI 邊界之 POD 結構（Phase 5 layout v3）。
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 128-bit 資產 GUID（與託管 VisionGal.Managed.Assets.GUID 對齊）。 */
typedef struct NNGuid
{
	std::uint64_t high;
	std::uint64_t low;
} NNGuid;

/** @brief 三維變換（位置 / 歐拉角 / 縮放，單位由實作定義）。 */
typedef struct NNTransform3
{
	float position[3];
	float rotation[3];
	float scale[3];
} NNTransform3;

#ifdef __cplusplus
} /* extern "C" */

/** @brief NNGuid 相等性比较。 */
inline bool operator==(const NNGuid& a, const NNGuid& b) noexcept
{
	return a.high == b.high && a.low == b.low;
}

/** @brief NNGuid 不等性比较。 */
inline bool operator!=(const NNGuid& a, const NNGuid& b) noexcept
{
	return !(a == b);
}
#endif
