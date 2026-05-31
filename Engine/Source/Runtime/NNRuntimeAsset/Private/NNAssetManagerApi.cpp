/**
 * @file NNAssetManagerApi.cpp
 * @brief NNAssetManagerAPI → NNAssetManager 橋接。
 *
 * 將 C ABI 函數表轉發至 NNAssetManager 單例。
 */

#include "NNAssetManager.h"
#include "NNNativeEngineAPI/Include/AssetManagerAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{

using NN::Runtime::Asset::NNAssetManager;

NNAssetHandle NN_ENGINE_ABI_STDCALL rt_am_loadAssetSync(NNGuid guid, std::uint64_t typeId)
{
	auto handle = NNAssetManager::Instance().LoadAssetSync(guid, typeId);
	return static_cast<NNAssetHandle>(handle.Value());
}

NNAsyncWaitHandle NN_ENGINE_ABI_STDCALL rt_am_loadAssetAsync(
	NNGuid guid,
	std::uint64_t typeId,
	NNLoadPriority priority,
	NNAssetLoadCompletedCallback callback,
	void* callbackUserData)
{
	return NNAssetManager::Instance().LoadAssetAsync(guid, typeId, priority, callback, callbackUserData);
}

void NN_ENGINE_ABI_STDCALL rt_am_unloadAsset(NNAssetHandle handle)
{
	NNAssetManager::Instance().UnloadAsset(static_cast<std::uint64_t>(handle));
}

void NN_ENGINE_ABI_STDCALL rt_am_unloadAssetByGuid(NNGuid guid)
{
	NNAssetManager::Instance().UnloadAssetByGuid(guid);
}

int NN_ENGINE_ABI_STDCALL rt_am_isAssetLoaded(NNAssetHandle handle)
{
	return NNAssetManager::Instance().IsLoaded(static_cast<std::uint64_t>(handle)) ? 1 : 0;
}

int NN_ENGINE_ABI_STDCALL rt_am_isAssetLoading(NNAssetHandle handle)
{
	return NNAssetManager::Instance().IsLoading(static_cast<std::uint64_t>(handle)) ? 1 : 0;
}

NNAssetHandle NN_ENGINE_ABI_STDCALL rt_am_getAssetByGuid(NNGuid guid)
{
	auto handle = NNAssetManager::Instance().GetLoadedAsset(guid);
	return static_cast<NNAssetHandle>(handle.Value());
}

NNGuid NN_ENGINE_ABI_STDCALL rt_am_getGuidByAsset(NNAssetHandle handle)
{
	return NNAssetManager::Instance().GetGuidByHandle(static_cast<std::uint64_t>(handle));
}

void NN_ENGINE_ABI_STDCALL rt_am_addRef(NNAssetHandle handle)
{
	NNAssetManager::Instance().AddRef(static_cast<std::uint64_t>(handle));
}

void NN_ENGINE_ABI_STDCALL rt_am_releaseRef(NNAssetHandle handle)
{
	NNAssetManager::Instance().ReleaseRef(static_cast<std::uint64_t>(handle));
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_am_getRefCount(NNAssetHandle handle)
{
	return NNAssetManager::Instance().GetRefCount(static_cast<std::uint64_t>(handle));
}

const void* NN_ENGINE_ABI_STDCALL rt_am_getAssetData(NNAssetHandle handle)
{
	return NNAssetManager::Instance().GetAssetData(static_cast<std::uint64_t>(handle));
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_am_getAssetDataSize(NNAssetHandle handle)
{
	return NNAssetManager::Instance().GetAssetDataSize(static_cast<std::uint64_t>(handle));
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_am_getBlobCount(NNAssetHandle handle)
{
	return NNAssetManager::Instance().GetBlobCount(static_cast<std::uint64_t>(handle));
}

const void* NN_ENGINE_ABI_STDCALL rt_am_getBlobData(NNAssetHandle handle, std::uint32_t index)
{
	return NNAssetManager::Instance().GetBlobData(static_cast<std::uint64_t>(handle), index);
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_am_getBlobSize(NNAssetHandle handle, std::uint32_t index)
{
	return NNAssetManager::Instance().GetBlobSize(static_cast<std::uint64_t>(handle), index);
}

int NN_ENGINE_ABI_STDCALL rt_am_mountPackage(const char* packPathUtf8)
{
	return NNAssetManager::Instance().MountPackage(packPathUtf8) ? 0 : -1;
}

void NN_ENGINE_ABI_STDCALL rt_am_unmountPackage(const char* packPathUtf8)
{
	NNAssetManager::Instance().UnmountPackage(packPathUtf8);
}

int NN_ENGINE_ABI_STDCALL rt_am_isAssetInPackage(NNGuid guid)
{
	return NNAssetManager::Instance().IsAssetInPackage(guid) ? 1 : 0;
}

void NN_ENGINE_ABI_STDCALL rt_am_markForReload(NNGuid guid)
{
	NNAssetManager::Instance().MarkForReload(guid);
}

void NN_ENGINE_ABI_STDCALL rt_am_reloadMarkedAssets()
{
	NNAssetManager::Instance().ReloadMarkedAssets();
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_am_getLoadedAssetCount()
{
	return NNAssetManager::Instance().GetLoadedAssetCount();
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_am_getTotalMemoryUsage()
{
	return NNAssetManager::Instance().GetTotalMemoryUsage();
}

int NN_ENGINE_ABI_STDCALL rt_am_initializeAssetManager(const char* projectRootUtf8)
{
	return NNAssetManager::Instance().Initialize(projectRootUtf8 ? projectRootUtf8 : "") ? 1 : 0;
}

} // namespace

extern "C" NN_ASSET_API void NNBuildAssetManagerRuntimeApi(NNAssetManagerAPI* api)
{
	if (api == nullptr)
		return;

	api->loadAssetSync = &rt_am_loadAssetSync;
	api->loadAssetAsync = &rt_am_loadAssetAsync;
	api->unloadAsset = &rt_am_unloadAsset;
	api->unloadAssetByGuid = &rt_am_unloadAssetByGuid;
	api->isAssetLoaded = &rt_am_isAssetLoaded;
	api->isAssetLoading = &rt_am_isAssetLoading;
	api->getAssetByGuid = &rt_am_getAssetByGuid;
	api->getGuidByAsset = &rt_am_getGuidByAsset;
	api->addRef = &rt_am_addRef;
	api->releaseRef = &rt_am_releaseRef;
	api->getRefCount = &rt_am_getRefCount;
	api->getAssetData = &rt_am_getAssetData;
	api->getAssetDataSize = &rt_am_getAssetDataSize;
	api->getBlobCount = &rt_am_getBlobCount;
	api->getBlobData = &rt_am_getBlobData;
	api->getBlobSize = &rt_am_getBlobSize;
	api->mountPackage = &rt_am_mountPackage;
	api->unmountPackage = &rt_am_unmountPackage;
	api->isAssetInPackage = &rt_am_isAssetInPackage;
	api->markForReload = &rt_am_markForReload;
	api->reloadMarkedAssets = &rt_am_reloadMarkedAssets;
	api->getLoadedAssetCount = &rt_am_getLoadedAssetCount;
	api->getTotalMemoryUsage = &rt_am_getTotalMemoryUsage;
	api->initializeAssetManager = &rt_am_initializeAssetManager;
}
