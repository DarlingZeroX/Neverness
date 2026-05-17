#pragma once

/**
 * @file EntityAPI.h
 * @brief **VGEntityAPI**：预留之 Native **ECS / 实体服务** 子函数表入口（Engine Service ABI 之一）。
 *
 * **与 SceneAPI 的边界（必读）**
 * - @ref VGEntityHandle（定义于 `EngineHandles.h`，经 `SceneAPI.h` 暴露）表示 **场景图 / Prefab 实例** 的不透明句柄，
 *   由 `VGSceneAPI` 的 spawn、destroy、层级与变换等函数操作。
 * - 托管程序集 **VisionGal.Managed.Entity** 中的 **EntityHandle**（Index + Generation）表示 **纯 C# 侧 ECS 首包**，
 *   与上述 `VGEntityHandle` **语义独立**；本阶段 **不承诺** 两种句柄在数值上可互转或自动同步。
 * - 本头文件定义的 **VGEntityAPI** 为 **独立子表**，挂载于 `VGNativeEngineAPI` 聚合体尾部，避免把未来 ECS 镜像能力
 *   塞进 `VGSceneAPI`，以降低场景运行时与 ECS 演进之间的耦合。
 *
 * **首包（骨架）约定**
 * - 暴露 `getServiceAbiToken`，供宿主与单测校验「子表已接线」；返回值 `VG_ENTITY_SERVICE_ABI_TOKEN` 为固定魔数。
 * - **layout v5** 起在子表尾部追加 `getRuntimeTick`，返回由 **VGEngineRuntime::EntitySubsystem** 驱动的单调帧计数，
 *   用于区分 **Stub 表**与 **`BuildRuntime` 覆写**后的 Runtime 路径（仍不代表与托管 **EntityWorld** 已数据镜像）。
 * - 后续若再增加函数指针，仅允许在本结构体 **尾部追加**；破坏性变更须递增 `VG_NATIVE_ENGINE_API_LAYOUT_VERSION`。
 */

#include <cstdint>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 子表接线验证用魔数（ASCII「VGEn」小端打包），与托管 `VGNativeEngineApiConstants.EntityServiceAbiToken` 对齐。
 */
#define VG_ENTITY_SERVICE_ABI_TOKEN 0x5647456Eu

/**
 * @brief 返回 `VG_ENTITY_SERVICE_ABI_TOKEN`；Stub 实现会累加 Stub 调用计数，便于测试观测。
 */
typedef std::uint32_t(VG_ENGINE_ABI_STDCALL* VGEntityGetServiceAbiTokenFn)(void);

/**
 * @brief 返回 **EntitySubsystem** 维护之单调 **`runtimeTick`**（Stub 恒为 **0**；Runtime 覆写后随 **Tick** 递增）。
 */
typedef std::uint64_t(VG_ENGINE_ABI_STDCALL* VGEntityGetRuntimeTickFn)(void);

typedef struct VGEntityAPI
{
	VGEntityGetServiceAbiTokenFn getServiceAbiToken;
	VGEntityGetRuntimeTickFn getRuntimeTick;
} VGEntityAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
