/**
 * @file AssetRegistryApiStubs.cpp
 * @brief **NNAssetRegistryAPI** Stub 函數指標，狀態委託 **AssetRegistryStubDatabase**。
 */

#include "AssetRegistry/AssetRegistryStubDatabase.h"
#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
int NN_ENGINE_ABI_STDCALL stub_reg_registerAsset(const char* virtualPathUtf8, NNGuid guid)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::RegisterAsset(virtualPathUtf8, guid);
}

int NN_ENGINE_ABI_STDCALL stub_reg_unregisterByGuid(NNGuid guid)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::UnregisterByGuid(guid);
}

int NN_ENGINE_ABI_STDCALL stub_reg_unregisterByPath(const char* virtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::UnregisterByPath(virtualPathUtf8);
}

int NN_ENGINE_ABI_STDCALL stub_reg_resolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::ResolvePathByGuid(guid, outUtf8, outCapacity);
}

int NN_ENGINE_ABI_STDCALL stub_reg_resolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::ResolveGuidByPath(virtualPathUtf8, outGuid);
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_reg_getDependencyCount(NNGuid guid)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::GetDependencyCount(guid);
}

int NN_ENGINE_ABI_STDCALL stub_reg_getDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::GetDependencyAt(guid, index, outDependency);
}

NNGuid NN_ENGINE_ABI_STDCALL stub_reg_importAsset(const char* virtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::AssetRegistry::ImportAsset(virtualPathUtf8);
}
} // namespace

extern "C" void NNBuildAssetRegistryApiStubs(NNAssetRegistryAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->registerAsset = &stub_reg_registerAsset;
	api->unregisterByGuid = &stub_reg_unregisterByGuid;
	api->unregisterByPath = &stub_reg_unregisterByPath;
	api->resolvePathByGuid = &stub_reg_resolvePathByGuid;
	api->resolveGuidByPath = &stub_reg_resolveGuidByPath;
	api->getDependencyCount = &stub_reg_getDependencyCount;
	api->getDependencyAt = &stub_reg_getDependencyAt;
	api->importAsset = &stub_reg_importAsset;
}
