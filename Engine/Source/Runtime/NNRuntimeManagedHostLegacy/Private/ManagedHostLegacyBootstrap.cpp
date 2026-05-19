/**
 * @file ManagedHostLegacyBootstrap.cpp
 * @brief Legacy Host：解析 Entry.Bootstrap / RuntimeTick 并注册 ManagedBridge 回调。
 */

#include "ManagedHost.h"

#include "ManagedRuntimeBridge.h"
#include "NativeAPI.h"

namespace
{
	using BootstrapFn = void (*)(void* nativeApiTable);
	using RuntimeTickFn = void (*)(float deltaTimeSeconds);

	const char kEntryTypeName[] = "Neverness.Managed.Runtime.Entry, Neverness.Runtime.Host";
}

bool VGManagedHost_BootstrapDefaultRuntime(
	VGManagedHost& host,
	const std::filesystem::path& hostAssemblyPath)
{
	void* bootstrapRaw = nullptr;
	if (!host.TryGetUnmanagedCallersOnly(
			hostAssemblyPath,
			kEntryTypeName,
			"Bootstrap",
			&bootstrapRaw))
	{
		return false;
	}

	void* tickRaw = nullptr;
	if (!host.TryGetUnmanagedCallersOnly(
			hostAssemblyPath,
			kEntryTypeName,
			"RuntimeTick",
			&tickRaw))
	{
		return false;
	}

	NNEngineRuntimeHost_SetManagedTickCallback(reinterpret_cast<NNManagedRuntimeTickFn>(tickRaw));

	const auto* table = NNNativeApi_GetDefaultTable();
	reinterpret_cast<BootstrapFn>(bootstrapRaw)(const_cast<void*>(static_cast<const void*>(table)));
	return true;
}
