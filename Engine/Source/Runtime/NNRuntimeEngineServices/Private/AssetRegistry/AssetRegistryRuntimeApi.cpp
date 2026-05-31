/**
 * @file AssetRegistryRuntimeApi.cpp
 * @brief NNAssetRegistryAPI Runtime 转发。
 *
 * 所有 16 个函数统一转发至 NNAssetRegistry::Instance()（NNRuntimeAsset 模块）。
 * 旧 AssetRegistrySubsystem 已删除，不再使用。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNAssetRegistry.h"

namespace
{
using NN::Runtime::Asset::NNAssetRegistry;

/* === GUID ↔ Path === */

int NN_ENGINE_ABI_STDCALL rt_reg_registerAsset(const char* virtualPathUtf8, NNGuid guid)
{
	return NNAssetRegistry::Instance().RegisterAsset(virtualPathUtf8, guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_unregisterByGuid(NNGuid guid)
{
	return NNAssetRegistry::Instance().UnregisterByGuid(guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_unregisterByPath(const char* virtualPathUtf8)
{
	return NNAssetRegistry::Instance().UnregisterByPath(virtualPathUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_reg_resolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity)
{
	return NNAssetRegistry::Instance().ResolvePathByGuid(guid, outUtf8, outCapacity);
}

int NN_ENGINE_ABI_STDCALL rt_reg_resolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid)
{
	return NNAssetRegistry::Instance().ResolveGuidByPath(virtualPathUtf8, outGuid);
}

/* === 依赖管理 === */

std::uint32_t NN_ENGINE_ABI_STDCALL rt_reg_getDependencyCount(NNGuid guid)
{
	return NNAssetRegistry::Instance().GetDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_getDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency)
{
	return NNAssetRegistry::Instance().GetDependencyAt(guid, index, outDependency);
}

NNGuid NN_ENGINE_ABI_STDCALL rt_reg_importAsset(const char* virtualPathUtf8)
{
	return NNAssetRegistry::Instance().ImportAsset(virtualPathUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_reg_setDependencies(NNGuid guid, const NNGuid* deps, std::uint32_t count)
{
	return NNAssetRegistry::Instance().SetDependencies(guid, deps, count);
}

int NN_ENGINE_ABI_STDCALL rt_reg_addDependency(NNGuid guid, NNGuid dependency)
{
	return NNAssetRegistry::Instance().AddDependency(guid, dependency);
}

int NN_ENGINE_ABI_STDCALL rt_reg_removeDependency(NNGuid guid, NNGuid dependency)
{
	return NNAssetRegistry::Instance().RemoveDependency(guid, dependency);
}

/* === 反向依赖 + 图查询 === */

std::uint32_t NN_ENGINE_ABI_STDCALL rt_reg_getReverseDependencyCount(NNGuid guid)
{
	return NNAssetRegistry::Instance().GetReverseDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_getReverseDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDep)
{
	return NNAssetRegistry::Instance().GetReverseDependencyAt(guid, index, outDep);
}

int NN_ENGINE_ABI_STDCALL rt_reg_hasCycle()
{
	return NNAssetRegistry::Instance().HasCycle();
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_reg_getAssetCount()
{
	return NNAssetRegistry::Instance().GetAssetCount();
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_reg_getEdgeCount()
{
	return NNAssetRegistry::Instance().GetEdgeCount();
}

} // namespace

extern "C" void NNBuildAssetRegistryRuntimeApi(NNAssetRegistryAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->registerAsset = &rt_reg_registerAsset;
	api->unregisterByGuid = &rt_reg_unregisterByGuid;
	api->unregisterByPath = &rt_reg_unregisterByPath;
	api->resolvePathByGuid = &rt_reg_resolvePathByGuid;
	api->resolveGuidByPath = &rt_reg_resolveGuidByPath;
	api->getDependencyCount = &rt_reg_getDependencyCount;
	api->getDependencyAt = &rt_reg_getDependencyAt;
	api->importAsset = &rt_reg_importAsset;
	api->setDependencies = &rt_reg_setDependencies;
	api->addDependency = &rt_reg_addDependency;
	api->removeDependency = &rt_reg_removeDependency;
	api->getReverseDependencyCount = &rt_reg_getReverseDependencyCount;
	api->getReverseDependencyAt = &rt_reg_getReverseDependencyAt;
	api->hasCycle = &rt_reg_hasCycle;
	api->getAssetCount = &rt_reg_getAssetCount;
	api->getEdgeCount = &rt_reg_getEdgeCount;
}
