/**
 * @file NNAssetCookerApi.cpp
 * @brief NNAssetCookerAPI C ABI 桥接。
 *
 * 管理构建清单的生命周期，转发 cook 调用到 NNAssetCooker。
 */

#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "NNNativeEngineAPI/Include/AssetCookerAPI.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNAssetCooker/Include/NNAssetCooker.h"
#include "NNAssetCooker/Include/NNCookManifest.h"

namespace
{

using NN::Runtime::Asset::NNAssetCooker;
using NN::Runtime::Asset::NNCookManifest;
using NN::Runtime::Asset::NNCookAssetEntry;
using NN::Runtime::Asset::NNCookGroup;
using NN::Runtime::Asset::NNCookResult;

/* 清单存储（句柄 → 清单） */
std::mutex g_manifestMutex;
std::uint64_t g_nextHandle = 1;
std::unordered_map<std::uint64_t, std::unique_ptr<NNCookManifest>> g_manifests;

NNCookManifest* GetManifest(std::uint64_t handle)
{
	std::lock_guard<std::mutex> lock(g_manifestMutex);
	auto it = g_manifests.find(handle);
	return it != g_manifests.end() ? it->second.get() : nullptr;
}

std::uint64_t NN_ENGINE_ABI_STDCALL cooker_createManifest()
{
	std::lock_guard<std::mutex> lock(g_manifestMutex);
	auto handle = g_nextHandle++;
	g_manifests[handle] = std::make_unique<NNCookManifest>();
	return handle;
}

void NN_ENGINE_ABI_STDCALL cooker_destroyManifest(std::uint64_t manifestHandle)
{
	std::lock_guard<std::mutex> lock(g_manifestMutex);
	g_manifests.erase(manifestHandle);
}

void NN_ENGINE_ABI_STDCALL cooker_setOutputRoot(std::uint64_t manifestHandle, const char* pathUtf8)
{
	auto* m = GetManifest(manifestHandle);
	if (m && pathUtf8)
		m->SetOutputRoot(pathUtf8);
}

void NN_ENGINE_ABI_STDCALL cooker_setLibraryRoot(std::uint64_t manifestHandle, const char* pathUtf8)
{
	auto* m = GetManifest(manifestHandle);
	if (m && pathUtf8)
		m->SetLibraryRoot(pathUtf8);
}

void NN_ENGINE_ABI_STDCALL cooker_addAsset(std::uint64_t manifestHandle,
	NNGuid guid, std::uint64_t typeId, const char* sourcePathUtf8, std::uint32_t groupIndex)
{
	auto* m = GetManifest(manifestHandle);
	if (!m) return;

	NNCookAssetEntry entry{};
	entry.guid = guid;
	entry.typeId = typeId;
	entry.sourcePath = sourcePathUtf8 ? sourcePathUtf8 : "";
	entry.groupIndex = groupIndex;
	m->AddAsset(entry);
}

void NN_ENGINE_ABI_STDCALL cooker_addGroup(std::uint64_t manifestHandle,
	const char* nameUtf8, const char* addressUtf8, std::uint32_t strategy, const char* outputPathUtf8)
{
	auto* m = GetManifest(manifestHandle);
	if (!m) return;

	NNCookGroup group{};
	group.name = nameUtf8 ? nameUtf8 : "";
	group.address = addressUtf8 ? addressUtf8 : "";
	group.strategy = strategy;
	group.outputPath = outputPathUtf8 ? outputPathUtf8 : "";
	m->AddGroup(group);
}

NNCookResultData NN_ENGINE_ABI_STDCALL cooker_cook(std::uint64_t manifestHandle)
{
	NNCookResultData resultData{};
	auto* m = GetManifest(manifestHandle);
	if (!m)
	{
		resultData.success = 0;
		return resultData;
	}

	NNAssetCooker cooker;
	auto result = cooker.Cook(*m);

	resultData.success = result.success ? 1 : 0;
	resultData.totalAssets = result.totalAssets;
	resultData.cookedAssets = result.cookedAssets;
	resultData.failedAssets = result.failedAssets;
	resultData.generatedPacks = result.generatedPacks;
	resultData.elapsedSeconds = result.elapsedSeconds;
	return resultData;
}

} // namespace

extern "C" void NNBuildAssetCookerRuntimeApi(NNAssetCookerAPI* api)
{
	if (api == nullptr)
		return;

	api->createManifest = &cooker_createManifest;
	api->destroyManifest = &cooker_destroyManifest;
	api->setOutputRoot = &cooker_setOutputRoot;
	api->setLibraryRoot = &cooker_setLibraryRoot;
	api->addAsset = &cooker_addAsset;
	api->addGroup = &cooker_addGroup;
	api->cook = &cooker_cook;
}
