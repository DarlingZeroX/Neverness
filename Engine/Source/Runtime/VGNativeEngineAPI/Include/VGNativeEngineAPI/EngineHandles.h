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

#include "VGNativeEngineAPI/NativeInterop.h"

typedef std::uint64_t VGTextureHandle;
typedef std::uint64_t VGRenderTargetHandle;
typedef std::uint64_t VGElementHandle;
typedef std::uint64_t VGAudioHandle;
typedef std::uint64_t VGAssetHandle;
typedef std::uint64_t VGAsyncWaitHandle;
