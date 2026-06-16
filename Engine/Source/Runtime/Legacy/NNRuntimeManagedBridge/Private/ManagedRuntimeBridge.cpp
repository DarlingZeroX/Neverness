/**
 * @file ManagedRuntimeBridge.cpp
 * @brief 托管 RuntimeTick 回调存储；对齐 MANAGED Runtime 主导权迁移 Phase 6a。
 */

#include "ManagedRuntimeBridge.h"

namespace
{
	NNManagedRuntimeTickFn g_managedTick = nullptr;
}

extern "C" void NNEngineRuntimeHost_SetManagedTickCallback(NNManagedRuntimeTickFn fn)
{
	g_managedTick = fn;
}

extern "C" void NNEngineRuntimeHost_ClearManagedTickCallback(void)
{
	g_managedTick = nullptr;
}

extern "C" void NNEngineRuntimeHost_TickManaged(float deltaTimeSeconds)
{
	if (g_managedTick != nullptr)
	{
		g_managedTick(deltaTimeSeconds);
	}
}
