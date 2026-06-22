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
#include "AsyncWaitAPI.h"
#include "AudioAPI.h"
#include "InputAPI.h"
#include "RenderAPI.h"
#include "VfsAPI.h"
#include "EventAPI.h"
#include "RenderAssetAPI.h"
#include "ViewportRenderAPI.h"
#include "ViewportSurfaceAPI.h"
#include "DiligentAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 当前发布之 NNNativeEngineAPI 内存布局版本（与托管 NNNativeEngineApiConstants.LayoutVersion 对齐）。v32：移除 EditorSceneAPI（已迁移至 C# Friflo ECS）。 */
#define NN_NATIVE_ENGINE_API_LAYOUT_VERSION 32u

typedef struct NNNativeEngineAPI
{
	std::uint32_t layoutVersion;
	std::uint32_t reserved0;
	NNRenderAPI render;
	NNAudioAPI audio;
	NNInputAPI input;
	//NNSceneAPI scene;
	//NNEditorSceneAPI editorScene; — 已移除：迁移至 C# Friflo ECS
	NNAsyncWaitAPI asyncWait;
	/** @brief Runtime Host 生命周期（SDL 子系统、事件泵、帧边界）；见 `ApplicationAPI.h`。 */
	NNApplicationAPI application;
	/** @brief 窗口子系统（多窗口、Native 句柄）；见 `WindowAPI.h`。 */
	NNWindowAPI window;
	/** @brief 虚拟文件系统（Phase 1 文本/二进制 IO）；见 `VfsAPI.h`。 */
	NNVfsAPI vfs;
	/** @brief 事件队列（Pull-Based Event Pump）；見 `EventAPI.h`。 */
	NNEventAPI events;
	/** @brief Render 資產 GPU 管理器（CPU Asset → GPU Resource、ImGui Handle）；見 `RenderAssetAPI.h`。 */
	NNRenderAssetAPI renderAsset;
	/** @brief 视口渲染 API（场景 → 离屏 Framebuffer → Texture ID）；見 `ViewportRenderAPI.h`。 */
	NNViewportRenderAPI viewportRender;
	/** @brief 视口 Surface API（SwapChain 生命周期管理 + RenderCommands）；見 `ViewportSurfaceAPI.h`。v23 新增，v29 新增 RenderViewportCommands。 */
	NNViewportSurfaceAPI viewportSurface;
	/** @brief Diligent 底层设备指针暴露；見 `DiligentAPI.h`。v28 新增。 */
	NNDiligentAPI diligent;
} NNNativeEngineAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
