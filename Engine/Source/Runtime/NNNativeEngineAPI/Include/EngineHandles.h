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
/** @brief 託管 VGObject 與 Native 子系統對應之不透明控制代碼。 */
typedef std::uint64_t NNObjectHandle;
