#pragma once

/**
 * @file MediaAssetTypes.h
 * @brief 媒体资产类型定义——替代 NNRuntimeAsset 中的 NNAudioTypeInfo / NNVideoTypeInfo。
 *
 * 原定义在 NNRuntimeAsset/Include/NNAssetFormat.h，已随 NNRuntimeAsset 移至 Legacy。
 * 此文件定义 NNRuntimeMedia 模块自用的等价类型。
 */

#include <cstdint>

namespace NN::Runtime::Media
{
	/// @brief 音频类型信息（原 NNAudioTypeInfo）。
	struct NNAudioTypeInfo
	{
		std::uint32_t sampleRate = 0;
		std::uint32_t channels = 0;
		std::uint64_t sampleCount = 0;
		std::uint32_t format = 0;          /* NNAudioCompressionFormat */
		std::uint32_t flags = 0;           /* NN_AUDIO_FLAG_* */
	};

	/// @brief 视频类型信息（原 NNVideoTypeInfo）。
	struct NNVideoTypeInfo
	{
		std::uint32_t width = 0;           /* 视频像素宽度 */
		std::uint32_t height = 0;          /* 视频像素高度 */
		std::uint32_t fpsNum = 0;          /* 帧率分子（如 30000） */
		std::uint32_t fpsDen = 0;          /* 帧率分母（如 1001） */
		std::uint64_t frameCount = 0;      /* 总帧数 */
		double       duration = 0.0;       /* 总时长（秒） */
		std::uint32_t codecId = 0;         /* NNVideoCodec */
		std::uint32_t flags = 0;           /* NN_VIDEO_FLAG_* */
		std::uint32_t audioSampleRate = 0; /* 音轨采样率（0=无音轨） */
		std::uint32_t audioChannels = 0;   /* 音轨声道数 */
	};

} // namespace NN::Runtime::Media
