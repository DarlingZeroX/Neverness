/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

/**
 * @file NNMediaCooker.cpp
 * @brief 媒体 Cooker 实现（骨架）。
 *
 * TODO: 集成 NNMediaCore 的 FFmpeg 封装实现以下函数：
 * - NNMediaProbeFile     → FfmpegContext + avformat_find_stream_info
 * - NNMediaExtractPCM    → FAudioDecoder 一次性解码模式
 * - NNMediaDecodeThumbnail → FVideoDecoder 单帧解码模式
 * - NNMediaBuildSeekTable → av_seek_frame 遍历关键帧
 * - NNMediaDecodeAllFrames → FVideoDecoder 全量解码
 */

#include "NNMediaCooker.h"
#include <cstdlib>
#include <cstring>

extern "C"
{

int NNMediaProbeFile(const char* /*filePath*/, NNMediaProbeResult* /*outResult*/)
{
	// TODO: 实现 — 使用 NNMediaCore::FfmpegContext
	return -1; // 未实现
}

int NNMediaExtractPCM(
	const char* /*filePath*/,
	std::uint32_t /*targetChannels*/,
	std::uint32_t /*targetSampleRate*/,
	std::uint8_t** /*outData*/,
	std::uint64_t* /*outSize*/)
{
	// TODO: 实现 — 使用 NNMediaCore::FAudioDecoder
	return -1;
}

int NNMediaDecodeThumbnail(
	const char* /*filePath*/,
	std::uint32_t /*maxWidth*/,
	std::uint8_t** /*outRGBA*/,
	std::uint32_t* /*outWidth*/,
	std::uint32_t* /*outHeight*/)
{
	// TODO: 实现 — 使用 NNMediaCore::FVideoDecoder
	return -1;
}

int NNMediaBuildAudioSeekTable(
	const char* /*filePath*/,
	std::uint8_t** /*outData*/,
	std::uint64_t* /*outSize*/)
{
	// TODO: 实现
	return -1;
}

int NNMediaBuildVideoSeekTable(
	const char* /*filePath*/,
	std::uint8_t** /*outData*/,
	std::uint64_t* /*outSize*/)
{
	// TODO: 实现
	return -1;
}

int NNMediaDecodeAllFrames(
	const char* /*filePath*/,
	std::uint32_t /*maxWidth*/,
	NNMediaFrameCallback /*callback*/,
	void* /*userData*/)
{
	// TODO: 实现
	return -1;
}

void NNMediaFreeBuffer(std::uint8_t* buffer)
{
	std::free(buffer);
}

} // extern "C"
