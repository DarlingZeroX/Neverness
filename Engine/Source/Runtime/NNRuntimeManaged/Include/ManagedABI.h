#pragma once

#include <cstdint>

#include "NNRuntimeManagedConfig.h"

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
typedef enum NNLogChannel : std::uint32_t
{
	NN_LOG_CHANNEL_General = 0,
	NN_LOG_CHANNEL_Engine = 1,
	NN_LOG_CHANNEL_Script = 2,
	NN_LOG_CHANNEL_Asset = 3,
	NN_LOG_CHANNEL__Reserved = 0xFFFFFFFFu
} NNLogChannel;

/**
 * @struct NNAbiPadding64
 * @brief 64-bit 占位填充，供未来在结构体尾追加版本化扩展块时对齐使用。
 */
typedef struct NNAbiPadding64
{
	std::uint64_t opaque0;
	std::uint64_t opaque1;
} NNAbiPadding64;

#ifdef __cplusplus
} /* extern "C" */
#endif
