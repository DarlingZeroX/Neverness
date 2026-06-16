#pragma once

/**
 * @file NNMediaCooker.h
 * @brief 媒体 Cooker：基于 FFmpeg 的媒体探测/解码/转码。
 *
 * 职责：
 * - 探测媒体文件元信息（NNMediaProbeFile）
 * - 提取 PCM 数据（NNMediaExtractPCM）
 * - 解码首帧缩略图（NNMediaDecodeThumbnail）
 * - 构建 Seek 表（NNMediaBuildSeekTable）
 *
 * 所有 FFmpeg 调用在此模块内完成，不暴露到上层。
 */

#include <cstdint>
#include "../NNRuntimeMediaAssetsExport.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

/// 媒体探测结果（56 字节）
typedef struct NNMediaProbeResult
{
	std::uint32_t mediaType;       // 0=audio, 1=video
	std::uint32_t width;           // 视频宽度（音频=0）
	std::uint32_t height;          // 视频高度（音频=0）
	std::uint32_t sampleRate;      // 音频采样率（视频=音轨采样率）
	std::uint32_t channels;        // 音频声道数
	double       duration;         // 时长（秒）
	std::uint32_t fpsNum;          // 帧率分子
	std::uint32_t fpsDen;          // 帧率分母
	std::uint64_t frameCount;      // 总帧数
	std::uint32_t hasAudio;        // 是否含音轨
	std::uint32_t hasAlpha;        // 是否含 Alpha 通道
	char          codecName[32];   // 编码器名称
} NNMediaProbeResult;

#pragma pack(pop)

/// 探测媒体文件元信息
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaProbeFile(const char* filePath, NNMediaProbeResult* outResult);

/// 提取 PCM 数据（调用者用 NNMediaFreeBuffer 释放）
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaExtractPCM(
	const char* filePath,
	std::uint32_t targetChannels,
	std::uint32_t targetSampleRate,
	std::uint8_t** outData,
	std::uint64_t* outSize);

/// 解码首帧缩略图（RGBA，调用者用 NNMediaFreeBuffer 释放）
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaDecodeThumbnail(
	const char* filePath,
	std::uint32_t maxWidth,
	std::uint8_t** outRGBA,
	std::uint32_t* outWidth,
	std::uint32_t* outHeight);

/// 构建音频 Seek 表
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaBuildAudioSeekTable(
	const char* filePath,
	std::uint8_t** outData,
	std::uint64_t* outSize);

/// 构建视频 Seek 表
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaBuildVideoSeekTable(
	const char* filePath,
	std::uint8_t** outData,
	std::uint64_t* outSize);

/// 全量解码视频帧回调
typedef void (*NNMediaFrameCallback)(
	const std::uint8_t* rgbaData,
	std::uint32_t width,
	std::uint32_t height,
	std::uint64_t frameIndex,
	void* userData);

/// 全量解码视频帧（非流式）
NN_RUNTIME_MEDIA_ASSETS_API int NNMediaDecodeAllFrames(
	const char* filePath,
	std::uint32_t maxWidth,
	NNMediaFrameCallback callback,
	void* userData);

/// 释放由上述函数分配的缓冲区
NN_RUNTIME_MEDIA_ASSETS_API void NNMediaFreeBuffer(std::uint8_t* buffer);

#ifdef __cplusplus
}
#endif
