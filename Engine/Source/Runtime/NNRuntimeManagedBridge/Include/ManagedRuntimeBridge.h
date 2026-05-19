#pragma once

/**
 * @file ManagedRuntimeBridge.h
 * @brief Native 与托管 Runtime Tick 桥接；不启动 CoreCLR（由 Legacy Host 或产品宿主负责）。
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 托管 Entry.RuntimeTick 函数签名（stdcall 由调用方保证）。 */
typedef void (*NNManagedRuntimeTickFn)(float deltaTimeSeconds);

/**
 * @brief 注册托管每帧回调；在 Entry.Bootstrap 之后由宿主或 Legacy Host 调用。
 */
void NNEngineRuntimeHost_SetManagedTickCallback(NNManagedRuntimeTickFn fn);

/** @brief 清除托管 Tick 回调（Shutdown 时）。 */
void NNEngineRuntimeHost_ClearManagedTickCallback(void);

/**
 * @brief 调用已注册的托管 Kernel Tick；未注册时为 no-op。
 */
void NNEngineRuntimeHost_TickManaged(float deltaTimeSeconds);

#ifdef __cplusplus
}
#endif
