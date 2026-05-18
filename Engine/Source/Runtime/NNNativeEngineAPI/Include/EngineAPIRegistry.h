#pragma once

#include <cstdint>

/**
 * @file EngineAPIRegistry.h
 * @brief 聚合所有 Engine Service 子表之 **NNNativeEngineAPI** 根結構與建表入口宣告。
 *
 * 版本：
 * - `layoutVersion` 對應 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION`；破壞性子表欄位變更時遞增。
 * - 擴充規則：僅能在各子表尾或本聚合體尾 **追加** 欄位；禁止重排既有欄位。
 */

#include "AssetAPI.h"
#include "AssetRegistryAPI.h"
#include "AsyncWaitAPI.h"
#include "EntityAPI.h"
#include "AudioAPI.h"
#include "InputAPI.h"
#include "ObjectAPI.h"
#include "RenderAPI.h"
#include "SceneAPI.h"
#include "TimingAPI.h"
#include "UIAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 當前發佈之 NNNativeEngineAPI 記憶體佈局版本（與託管 `NNNativeEngineApiConstants.LayoutVersion` 對齊）。 */
#define NN_NATIVE_ENGINE_API_LAYOUT_VERSION 5u

typedef struct NNNativeEngineAPI
{
	std::uint32_t layoutVersion;
	std::uint32_t reserved0;
	NNRenderAPI render;
	NNUIAPI ui;
	NNAudioAPI audio;
	NNAssetAPI asset;
	NNInputAPI input;
	NNSceneAPI scene;
	NNTimingAPI timing;
	NNAsyncWaitAPI asyncWait;
	NNObjectAPI object;
	NNAssetRegistryAPI assetRegistry;
	/** @brief Native ECS／實體服務子表（骨架）；與 `NNSceneAPI` 使用之 `NNEntityHandle` 語意分離，見 `EntityAPI.h`。 */
	NNEntityAPI entity;
} NNNativeEngineAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
