#pragma once

/**
 * @file EngineHandles.h
 * @brief 以 **uint64** 為主的引擎資源控制代碼（Handle），禁止將 C++ 物件指標直接暴露給託管層。
 *
 * 契約（Phase 3 Stub）：
 * - 數值 **0** 表示無效或未建立；非零不保證已掛載真實 GPU/音訊資源（Stub 階段可能為合成 id）。
 * - Handle 由對應子系統分配與回收；跨執行緒使用規則由未來 Adapter 文件化，Stub 實作僅保證單執行緒測試安全。
 */

#include <cstdint>

#include "NativeInterop.h"

typedef std::uint64_t NNTextureHandle;
typedef std::uint64_t NNRenderTargetHandle;
typedef std::uint64_t NNElementHandle;
typedef std::uint64_t NNAudioHandle;
typedef std::uint64_t NNAssetHandle;
typedef std::uint64_t NNAsyncWaitHandle;
/**
 * @brief 場景實體 / Prefab 實例之不透明控制代碼（非 UI Element）。
 *
 * **邊界**：與託管 **VisionGal.Managed.Entity.EntityHandle**（純 C# ECS）語意獨立；勿與 `NNNativeEngineAPI::entity`
 *（`EntityAPI.h` 子表，預留 Native ECS）混淆——後者首包僅含服務魔數校驗，本句柄仍專屬場景子系統。
 */
typedef std::uint64_t NNEntityHandle;
/** @brief NNEntity 全局别名——与 NNEntityHandle 为同一类型，收口实体句柄定义。 */
using NNEntity = NNEntityHandle;
/** @brief 不透明场景 ID，0 = Invalid（原 SceneAPI.h，现收口至 EngineHandles）。 */
typedef std::uint64_t NNSceneHandle;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 场景操作结果码（原 SceneAPI.h，现收口至 EngineHandles）。 */
typedef enum NNSceneResult
{
	NN_SCENE_OK               = 0, ///< 成功
	NN_SCENE_ERR_NOT_FOUND    = 1, ///< 实体 / 场景未找到
	NN_SCENE_ERR_INVALID      = 2, ///< 句柄无效（0 或世代不匹配）
	NN_SCENE_ERR_BUFFER_SMALL = 3, ///< 输出缓冲区容量不足
	NN_SCENE_ERR_IO           = 4, ///< 序列化 / 反序列化 I/O 错误
} NNSceneResult;

/** @brief 资产加载优先级（原 AssetManagerAPI.h，现收口至 EngineHandles）。 */
typedef enum NNLoadPriority
{
	NN_LOAD_PRIORITY_CRITICAL   = 0,  ///< 立即需要（UI 纹理等）
	NN_LOAD_PRIORITY_HIGH       = 1,  ///< 相机附近资源
	NN_LOAD_PRIORITY_NORMAL     = 2,  ///< 默认
	NN_LOAD_PRIORITY_LOW        = 3,  ///< 后台预载
	NN_LOAD_PRIORITY_BACKGROUND = 4   ///< 最低优先级
} NNLoadPriority;

/** @brief 资产加载完成回调（原 AssetManagerAPI.h，现收口至 EngineHandles）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNAssetLoadCompletedCallback)(
	NNAssetHandle handle,
	int result,
	void* userData);

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @brief 託管 VGObject 與 Native 子系統對應之不透明控制代碼。 */
typedef std::uint64_t NNObjectHandle;
