/**
 * @file NNAssetRegistryApi.cpp
 * @brief NNAssetRegistryAPI 增強版 Runtime 轉發。
 *
 * 在原有 8 個函數基礎上新增：setDependencies, addDependency, removeDependency,
 * getReverseDependencyCount, getReverseDependencyAt, hasCycle, getAssetCount, getEdgeCount。
 */

#include "NNAssetRegistry.h"
#include "NNNativeEngineAPI/Include/AssetRegistryAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{

using NN::Runtime::Asset::NNAssetRegistry;

/* === 原有函數 === */

int NN_ENGINE_ABI_STDCALL reg_registerAsset(const char* virtualPathUtf8, NNGuid guid)
{
	return NNAssetRegistry::Instance().RegisterAsset(virtualPathUtf8, guid);
}

int NN_ENGINE_ABI_STDCALL reg_unregisterByGuid(NNGuid guid)
{
	return NNAssetRegistry::Instance().UnregisterByGuid(guid);
}

int NN_ENGINE_ABI_STDCALL reg_unregisterByPath(const char* virtualPathUtf8)
{
	return NNAssetRegistry::Instance().UnregisterByPath(virtualPathUtf8);
}

int NN_ENGINE_ABI_STDCALL reg_resolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity)
{
	return NNAssetRegistry::Instance().ResolvePathByGuid(guid, outUtf8, outCapacity);
}

int NN_ENGINE_ABI_STDCALL reg_resolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid)
{
	return NNAssetRegistry::Instance().ResolveGuidByPath(virtualPathUtf8, outGuid);
}

std::uint32_t NN_ENGINE_ABI_STDCALL reg_getDependencyCount(NNGuid guid)
{
	return NNAssetRegistry::Instance().GetDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL reg_getDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency)
{
	return NNAssetRegistry::Instance().GetDependencyAt(guid, index, outDependency);
}

NNGuid NN_ENGINE_ABI_STDCALL reg_importAsset(const char* virtualPathUtf8)
{
	return NNAssetRegistry::Instance().ImportAsset(virtualPathUtf8);
}

/* === 新增函數 === */

int NN_ENGINE_ABI_STDCALL reg_setDependencies(NNGuid guid, const NNGuid* deps, std::uint32_t count)
{
	return NNAssetRegistry::Instance().SetDependencies(guid, deps, count);
}

int NN_ENGINE_ABI_STDCALL reg_addDependency(NNGuid guid, NNGuid dependency)
{
	return NNAssetRegistry::Instance().AddDependency(guid, dependency);
}

int NN_ENGINE_ABI_STDCALL reg_removeDependency(NNGuid guid, NNGuid dependency)
{
	return NNAssetRegistry::Instance().RemoveDependency(guid, dependency);
}

std::uint32_t NN_ENGINE_ABI_STDCALL reg_getReverseDependencyCount(NNGuid guid)
{
	return NNAssetRegistry::Instance().GetReverseDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL reg_getReverseDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDep)
{
	return NNAssetRegistry::Instance().GetReverseDependencyAt(guid, index, outDep);
}

int NN_ENGINE_ABI_STDCALL reg_hasCycle()
{
	return NNAssetRegistry::Instance().HasCycle();
}

std::uint32_t NN_ENGINE_ABI_STDCALL reg_getAssetCount()
{
	return NNAssetRegistry::Instance().GetAssetCount();
}

std::uint32_t NN_ENGINE_ABI_STDCALL reg_getEdgeCount()
{
	return NNAssetRegistry::Instance().GetEdgeCount();
}

} // namespace

extern "C" void NNBuildAssetRegistryEnhancedRuntimeApi(NNAssetRegistryAPI* api)
{
	if (api == nullptr)
		return;

	/* 原有欄位 */
	api->registerAsset = &reg_registerAsset;
	api->unregisterByGuid = &reg_unregisterByGuid;
	api->unregisterByPath = &reg_unregisterByPath;
	api->resolvePathByGuid = &reg_resolvePathByGuid;
	api->resolveGuidByPath = &reg_resolveGuidByPath;
	api->getDependencyCount = &reg_getDependencyCount;
	api->getDependencyAt = &reg_getDependencyAt;
	api->importAsset = &reg_importAsset;

	/* Phase 1 新增欄位 */
	api->setDependencies = &reg_setDependencies;
	api->addDependency = &reg_addDependency;
	api->removeDependency = &reg_removeDependency;
	api->getReverseDependencyCount = &reg_getReverseDependencyCount;
	api->getReverseDependencyAt = &reg_getReverseDependencyAt;
	api->hasCycle = &reg_hasCycle;
	api->getAssetCount = &reg_getAssetCount;
	api->getEdgeCount = &reg_getEdgeCount;
}
