#pragma once

/**
 * @file MediaRuntimeAPI.h
 * @brief 媒体运行时 C ABI 函数指针表（供 C# P/Invoke 调用）。
 */

#include <cstdint>
#include "../NNRuntimeMediaExport.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

/// 媒体运行时 API 函数指针表
typedef struct NNMediaRuntimeAPI
{
	// ── 音频 ──
	void* (*createAudioPlayer)();
	void  (*destroyAudioPlayer)(void* player);
	int   (*audioPlayerPlay)(void* player, std::uint64_t assetHandle);
	void  (*audioPlayerStop)(void* player);
	void  (*audioPlayerPause)(void* player);
	void  (*audioPlayerSetVolume)(void* player, float volume);
	void  (*audioPlayerSetLoop)(void* player, int loop);
	double(*audioPlayerGetTime)(void* player);
	int   (*audioPlayerIsPlaying)(void* player);

	// ── 视频 ──
	void* (*createVideoPlayer)();
	void  (*destroyVideoPlayer)(void* player);
	int   (*videoPlayerPlay)(void* player, std::uint64_t assetHandle);
	void  (*videoPlayerStop)(void* player);
	void  (*videoPlayerUpdate)(void* player, float deltaTime);
	void  (*videoPlayerSetVolume)(void* player, float volume);
	void  (*videoPlayerSetLoop)(void* player, int loop);
	std::uint64_t(*videoPlayerGetTextureHandle)(void* player);
	double(*videoPlayerGetTime)(void* player);
	int   (*videoPlayerIsPlaying)(void* player);
	int   (*videoPlayerIsFinished)(void* player);

	// ── 视频纹理 ──
	std::uint64_t(*createVideoTexture)(std::uint32_t width, std::uint32_t height);
	void  (*updateVideoTexture)(std::uint64_t cacheKey, const std::uint8_t* rgba, std::uint32_t w, std::uint32_t h);
	void  (*releaseVideoTexture)(std::uint64_t cacheKey);

} NNMediaRuntimeAPI;

#pragma pack(pop)

/// 填充媒体运行时 API 表
NN_RUNTIME_MEDIA_API void NNBuildMediaRuntimeApi(NNMediaRuntimeAPI* api);

#ifdef __cplusplus
}
#endif
