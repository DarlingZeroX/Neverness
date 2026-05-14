#pragma once

#include <cstdint>

#include "VGManagedCoreConfig.h"

/**
 * @file ManagedABI.h
 * @brief 未来跨边界共享的 **小型 POD / 枚举**；Phase 2 仅定义日志通道枚举与占位。
 *
 * 禁止在此放入大体量 gameplay struct（对白、选项树等），以免 ABI 未稳定前过度膨胀。
 */

#ifdef __cplusplus
extern "C" {
#endif

/** 日志通道（扩展用）：当前 DefaultLogInfo 未区分通道，预留枚举值。 */
typedef enum VGLogChannel : std::uint32_t
{
	VG_LOG_CHANNEL_General = 0,
	VG_LOG_CHANNEL_Engine = 1,
	VG_LOG_CHANNEL_Script = 2,
	VG_LOG_CHANNEL_Asset = 3,
	VG_LOG_CHANNEL__Reserved = 0xFFFFFFFFu
} VGLogChannel;

/**
 * @struct VGAbiPadding64
 * @brief 64-bit 占位填充，供未来在结构体尾追加版本化扩展块时对齐使用。
 */
typedef struct VGAbiPadding64
{
	std::uint64_t opaque0;
	std::uint64_t opaque1;
} VGAbiPadding64;

#ifdef __cplusplus
} /* extern "C" */
#endif
