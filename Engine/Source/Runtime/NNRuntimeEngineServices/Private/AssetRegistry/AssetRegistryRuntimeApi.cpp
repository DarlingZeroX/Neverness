/**
 * @file AssetRegistryRuntimeApi.cpp
 * @brief **NNAssetRegistryAPI** Runtime 轉發至 **NNEngineRuntime::AssetRegistry()**。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

int NN_ENGINE_ABI_STDCALL rt_reg_registerAsset(const char* virtualPathUtf8, NNGuid guid)
{
	return NNEngineRuntime::Instance().AssetRegistry().RegisterAsset(virtualPathUtf8, guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_unregisterByGuid(NNGuid guid)
{
	return NNEngineRuntime::Instance().AssetRegistry().UnregisterByGuid(guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_unregisterByPath(const char* virtualPathUtf8)
{
	return NNEngineRuntime::Instance().AssetRegistry().UnregisterByPath(virtualPathUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_reg_resolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity)
{
	return NNEngineRuntime::Instance().AssetRegistry().ResolvePathByGuid(guid, outUtf8, outCapacity);
}

int NN_ENGINE_ABI_STDCALL rt_reg_resolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid)
{
	return NNEngineRuntime::Instance().AssetRegistry().ResolveGuidByPath(virtualPathUtf8, outGuid);
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_reg_getDependencyCount(NNGuid guid)
{
	return NNEngineRuntime::Instance().AssetRegistry().GetDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL rt_reg_getDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency)
{
	return NNEngineRuntime::Instance().AssetRegistry().GetDependencyAt(guid, index, outDependency);
}

NNGuid NN_ENGINE_ABI_STDCALL rt_reg_importAsset(const char* virtualPathUtf8)
{
	return NNEngineRuntime::Instance().AssetRegistry().ImportAsset(virtualPathUtf8);
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
}
