/**
 * @file AssetCookerApiStubs.cpp
 * @brief NNAssetCookerAPI 空桩（未接線時回傳 0 / no-op）。
 */

#include <cstring>

#include "NNNativeEngineAPI/Include/AssetCookerAPI.h"

namespace
{

std::uint64_t NN_ENGINE_ABI_STDCALL stub_createManifest()
{
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_destroyManifest(std::uint64_t)
{
}

void NN_ENGINE_ABI_STDCALL stub_setOutputRoot(std::uint64_t, const char*)
{
}

void NN_ENGINE_ABI_STDCALL stub_setLibraryRoot(std::uint64_t, const char*)
{
}

void NN_ENGINE_ABI_STDCALL stub_addAsset(std::uint64_t, NNGuid, std::uint64_t, const char*, std::uint32_t)
{
}

void NN_ENGINE_ABI_STDCALL stub_addGroup(std::uint64_t, const char*, const char*, std::uint32_t, const char*)
{
}

NNCookResultData NN_ENGINE_ABI_STDCALL stub_cook(std::uint64_t)
{
	NNCookResultData result{};
	result.success = 0;
	return result;
}

} // namespace

extern "C" void NNBuildAssetCookerApiStubs(NNAssetCookerAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	api->createManifest = &stub_createManifest;
	api->destroyManifest = &stub_destroyManifest;
	api->setOutputRoot = &stub_setOutputRoot;
	api->setLibraryRoot = &stub_setLibraryRoot;
	api->addAsset = &stub_addAsset;
	api->addGroup = &stub_addGroup;
	api->cook = &stub_cook;
}
