#pragma once

#include <cstdint>

/**
 * @file EngineAPIRegistry.h
 * @brief 聚合所有 Engine Service 子表之 **VGNativeEngineAPI** 根結構與建表入口宣告。
 *
 * 版本：
 * - `layoutVersion` 對應 `VG_NATIVE_ENGINE_API_LAYOUT_VERSION`；破壞性子表欄位變更時遞增。
 * - 擴充規則：僅能在各子表尾或本聚合體尾 **追加** 欄位；禁止重排既有欄位。
 */

#include "VGNativeEngineAPI/AssetAPI.h"
#include "VGNativeEngineAPI/AsyncWaitAPI.h"
#include "VGNativeEngineAPI/AudioAPI.h"
#include "VGNativeEngineAPI/InputAPI.h"
#include "VGNativeEngineAPI/RenderAPI.h"
#include "VGNativeEngineAPI/SceneAPI.h"
#include "VGNativeEngineAPI/TimingAPI.h"
#include "VGNativeEngineAPI/UIAPI.h"
#include "VGNativeEngineAPI/VGNativeEngineApiConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 當前發佈之 VGNativeEngineAPI 記憶體佈局版本（與託管 `VGNativeEngineApiConstants.LayoutVersion` 對齊）。 */
#define VG_NATIVE_ENGINE_API_LAYOUT_VERSION 1u

typedef struct VGNativeEngineAPI
{
	std::uint32_t layoutVersion;
	std::uint32_t reserved0;
	VGRenderAPI render;
	VGUIAPI ui;
	VGAudioAPI audio;
	VGAssetAPI asset;
	VGInputAPI input;
	VGSceneAPI scene;
	VGTimingAPI timing;
	VGAsyncWaitAPI asyncWait;
} VGNativeEngineAPI;

/**
 * @brief 將 `outTable` 清零後填入預設 Stub 函數指標（不連結 VGEngine）。
 */
VG_NATIVE_ENGINE_API void VGNativeEngineApiTable_BuildDefault(VGNativeEngineAPI* outTable);

/**
 * @brief 供測試斷言：累計 Stub 實作被呼叫次數（跨子系統加總）。
 */
VG_NATIVE_ENGINE_API std::uint32_t VGNativeEngineApi_GetStubInvokeCount(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
