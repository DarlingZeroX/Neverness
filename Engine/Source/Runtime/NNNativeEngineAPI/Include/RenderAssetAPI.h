#pragma once

/**
 * @file RenderAssetAPI.h
 * @brief Render 資產 GPU 管理器 ABI 函數表（CPU Asset → GPU Resource）。
 *
 * 與 RenderAPI.h（舊版紋理建立）分離；本表為 NNRuntimeRenderAssets 模組之完整介面。
 * 所有函數使用 __stdcall（Windows）；未接線時回傳 0 / no-op。
 *
 * v20 新增子表。
 */

#include "EngineTypes.h"
#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NNRenderAssetAPI
{
    /** @brief 取得 Texture 的 ImGui 兼容 Handle（後端無關）。textureAssetHandle 無效回傳 0。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *getImGuiTextureHandle)(std::uint64_t textureAssetHandle);

    /** @brief 從 RGBA8 像素建立 GPU Texture，回傳快取 key（0 = 失敗）。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *createTextureFromPixels)(
        std::uint32_t width,
        std::uint32_t height,
        const std::uint8_t* pixels,
        std::size_t pixelSize,
        int isSRGB);

    /** @brief 釋放 GPU Texture 資源。 */
    void(NN_ENGINE_ABI_STDCALL *releaseTexture)(std::uint64_t textureKey);

    /** @brief 重載 Texture（Hot Reload）。 */
    void(NN_ENGINE_ABI_STDCALL *reloadTextureFromPixels)(
        std::uint64_t textureKey,
        std::uint32_t width,
        std::uint32_t height,
        const std::uint8_t* pixels,
        std::size_t pixelSize,
        int isSRGB);

    /** @brief 取得 Texture 尺寸（1=成功, 0=失敗/未載入）。 */
    int(NN_ENGINE_ABI_STDCALL *getTextureDesc)(
        std::uint64_t textureKey,
        std::uint32_t* outWidth,
        std::uint32_t* outHeight);

    /** @brief Texture 是否已載入到 GPU（1=Resident, 0=未載入/驅逐）。 */
    int(NN_ENGINE_ABI_STDCALL *isTextureResident)(std::uint64_t textureKey);

    /** @brief 取得已快取 Texture 數量。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *getCachedTextureCount)(void);

    /** @brief 取得 GPU 紋理總記憶體使用量（位元組）。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *getTotalGPUMemory)(void);

    /** @brief 從已載入的 .nnasset 資源句柄建立 GPU Texture（讀 blob[0] 反序列化）。回傳快取 key（0 = 失敗）。guidLow 非零時建立 GUID→cacheKey 索引。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *loadTextureFromAsset)(std::uint64_t assetHandle, std::uint64_t guidLow);

    /** @brief 從已解析的 blob 資料直接建立 GPU Texture（避免跨模組單例問題）。回傳快取 key（0 = 失敗）。 */
    std::uint64_t(NN_ENGINE_ABI_STDCALL *loadTextureFromBlob)(
        const void* typeInfoData,
        std::uint64_t typeInfoSize,
        const void* pixelData,
        std::uint64_t pixelDataSize,
        std::uint64_t guidLow);

} NNRenderAssetAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
