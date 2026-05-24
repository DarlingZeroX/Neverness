/**
 * @file AssetManagerApiStubs.cpp
 * @brief NNAssetManagerAPI Stub 函數指標（全部回傳 0 / nullptr / no-op）。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{

NNAssetHandle NN_ENGINE_ABI_STDCALL stub_am_loadAssetSync(NNGuid, std::uint64_t)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

NNAsyncWaitHandle NN_ENGINE_ABI_STDCALL stub_am_loadAssetAsync(
	NNGuid, std::uint64_t, NNLoadPriority, NNAssetLoadCompletedCallback, void*)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_am_unloadAsset(NNAssetHandle) { NN::StubRuntime::BumpInvokeCount(); }
void NN_ENGINE_ABI_STDCALL stub_am_unloadAssetByGuid(NNGuid) { NN::StubRuntime::BumpInvokeCount(); }

int NN_ENGINE_ABI_STDCALL stub_am_isAssetLoaded(NNAssetHandle) { return 0; }
int NN_ENGINE_ABI_STDCALL stub_am_isAssetLoading(NNAssetHandle) { return 0; }

NNAssetHandle NN_ENGINE_ABI_STDCALL stub_am_getAssetByGuid(NNGuid) { return 0; }
NNGuid NN_ENGINE_ABI_STDCALL stub_am_getGuidByAsset(NNAssetHandle) { NNGuid z{}; return z; }

void NN_ENGINE_ABI_STDCALL stub_am_addRef(NNAssetHandle) { }
void NN_ENGINE_ABI_STDCALL stub_am_releaseRef(NNAssetHandle) { }
std::uint32_t NN_ENGINE_ABI_STDCALL stub_am_getRefCount(NNAssetHandle) { return 0; }

const void* NN_ENGINE_ABI_STDCALL stub_am_getAssetData(NNAssetHandle) { return nullptr; }
std::uint64_t NN_ENGINE_ABI_STDCALL stub_am_getAssetDataSize(NNAssetHandle) { return 0; }

std::uint32_t NN_ENGINE_ABI_STDCALL stub_am_getBlobCount(NNAssetHandle) { return 0; }
const void* NN_ENGINE_ABI_STDCALL stub_am_getBlobData(NNAssetHandle, std::uint32_t) { return nullptr; }
std::uint64_t NN_ENGINE_ABI_STDCALL stub_am_getBlobSize(NNAssetHandle, std::uint32_t) { return 0; }

int NN_ENGINE_ABI_STDCALL stub_am_mountPackage(const char*) { return -1; }
void NN_ENGINE_ABI_STDCALL stub_am_unmountPackage(const char*) { }
int NN_ENGINE_ABI_STDCALL stub_am_isAssetInPackage(NNGuid) { return 0; }

void NN_ENGINE_ABI_STDCALL stub_am_markForReload(NNGuid) { }
void NN_ENGINE_ABI_STDCALL stub_am_reloadMarkedAssets() { }

std::uint64_t NN_ENGINE_ABI_STDCALL stub_am_getLoadedAssetCount() { return 0; }
std::uint64_t NN_ENGINE_ABI_STDCALL stub_am_getTotalMemoryUsage() { return 0; }

} // namespace

extern "C" void NNBuildAssetManagerApiStubs(NNAssetManagerAPI* api)
{
	if (api == nullptr)
		return;

	api->loadAssetSync = &stub_am_loadAssetSync;
	api->loadAssetAsync = &stub_am_loadAssetAsync;
	api->unloadAsset = &stub_am_unloadAsset;
	api->unloadAssetByGuid = &stub_am_unloadAssetByGuid;
	api->isAssetLoaded = &stub_am_isAssetLoaded;
	api->isAssetLoading = &stub_am_isAssetLoading;
	api->getAssetByGuid = &stub_am_getAssetByGuid;
	api->getGuidByAsset = &stub_am_getGuidByAsset;
	api->addRef = &stub_am_addRef;
	api->releaseRef = &stub_am_releaseRef;
	api->getRefCount = &stub_am_getRefCount;
	api->getAssetData = &stub_am_getAssetData;
	api->getAssetDataSize = &stub_am_getAssetDataSize;
	api->getBlobCount = &stub_am_getBlobCount;
	api->getBlobData = &stub_am_getBlobData;
	api->getBlobSize = &stub_am_getBlobSize;
	api->mountPackage = &stub_am_mountPackage;
	api->unmountPackage = &stub_am_unmountPackage;
	api->isAssetInPackage = &stub_am_isAssetInPackage;
	api->markForReload = &stub_am_markForReload;
	api->reloadMarkedAssets = &stub_am_reloadMarkedAssets;
	api->getLoadedAssetCount = &stub_am_getLoadedAssetCount;
	api->getTotalMemoryUsage = &stub_am_getTotalMemoryUsage;
}
