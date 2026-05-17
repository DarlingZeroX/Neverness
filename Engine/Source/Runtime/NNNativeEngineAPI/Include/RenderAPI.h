#pragma once

/**
 * @file RenderAPI.h
 * @brief 渲染相關 **Engine Service** 函數表（僅函數指標聚合，不含實作）。
 *
 * 執行緒：Stub 與未來 Adapter 預設假設由 **主執行緒 / RHI 執行緒** 之一單點呼叫；並發呼叫行為未定義，須由實作端加鎖。
 */

#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 建立可上傳像素資料之紋理；失敗回傳 0。 */
typedef VGTextureHandle(VG_ENGINE_ABI_STDCALL* VGRenderCreateTextureFn)(std::uint32_t width, std::uint32_t height);

/**
 * @brief 上傳像素至既有紋理。
 * @param texture 必須為由 createTexture 取得之有效 Handle；無效則 no-op。
 * @param pixelBytes 可為 nullptr（視為 no-op）；否則指向連續位元組緩衝。
 * @param byteCount 緩衝長度；與格式/尺寸不符時實作端可截斷或拒絕（Stub 忽略）。
 */
typedef void(VG_ENGINE_ABI_STDCALL* VGRenderUploadTextureFn)(
	VGTextureHandle texture,
	const std::uint8_t* pixelBytes,
	std::size_t byteCount);

/** @brief 建立渲染目標；失敗回傳 0。 */
typedef VGRenderTargetHandle(VG_ENGINE_ABI_STDCALL* VGRenderCreateRenderTargetFn)(std::uint32_t width, std::uint32_t height);

typedef struct VGRenderAPI
{
	VGRenderCreateTextureFn createTexture;
	VGRenderUploadTextureFn uploadTexture;
	VGRenderCreateRenderTargetFn createRenderTarget;
} VGRenderAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
