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

#include "ApplicationAPI.h"
#include "WindowAPI.h"
#include "AssetAPI.h"
#include "AssetManagerAPI.h"
#include "AssetRegistryAPI.h"
#include "AssetCookerAPI.h"
#include "AsyncWaitAPI.h"
#include "EditorSceneAPI.h"
#include "EntityAPI.h"
#include "AudioAPI.h"
#include "InputAPI.h"
#include "ObjectAPI.h"
#include "RenderAPI.h"
#include "SceneAPI.h"
#include "TimingAPI.h"
#include "UIAPI.h"
#include "VfsAPI.h"
#include "EventAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 當前發佈之 NNNativeEngineAPI 記憶體佈局版本（與託管 `NNNativeEngineApiConstants.LayoutVersion` 對齊）。v19：新增 NNEventAPI 子表（Pull-Based 事件队列）。 */
#define NN_NATIVE_ENGINE_API_LAYOUT_VERSION 19u

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
	/** @brief Editor 专用场景查询子表（独立 ABI，layoutVersion = 2）。见 EditorSceneAPI.h。 */
	NNEditorSceneAPI editorScene;
	NNTimingAPI timing;
	NNAsyncWaitAPI asyncWait;
	NNObjectAPI object;
	NNAssetRegistryAPI assetRegistry;
	/** @brief Native ECS／實體服務子表（骨架）；與 `NNSceneAPI` 使用之 `NNEntityHandle` 語意分離，見 `EntityAPI.h`。 */
	NNEntityAPI entity;
	/** @brief Runtime Host 生命周期（SDL 子系统、事件泵、帧边界）；见 `ApplicationAPI.h`。 */
	NNApplicationAPI application;
	/** @brief 窗口子系统（多窗口、Native 句柄）；见 `WindowAPI.h`。 */
	NNWindowAPI window;
	/** @brief 虚拟文件系统（Phase 1 文本/二进制 IO）；见 `VfsAPI.h`。 */
	NNVfsAPI vfs;
	/** @brief Runtime 資產管理器（同步/異步載入、卸載、包管理、Hot Reload）；見 `AssetManagerAPI.h`。 */
	NNAssetManagerAPI assetManager;
	/** @brief 資產編譯/打包器（.nnpack 構建）；見 `AssetCookerAPI.h`。 */
	NNAssetCookerAPI assetCooker;
	/** @brief 事件队列（Pull-Based Event Pump）；見 `EventAPI.h`。 */
	NNEventAPI events;
} NNNativeEngineAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
