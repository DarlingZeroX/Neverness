/**
 * @file RenderAssetRuntimeApi.cpp
 * @brief **NNRenderAssetAPI** Runtime 桥接：将函数指针转发至 **NNRenderAssetManager**。
 */

#include <iostream>

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeRenderAssets/Include/NNRenderAssetManager.h"
#include "NNRuntimeRenderAssets/Include/NNTextureSourceAsset.h"

namespace
{
using NN::Runtime::Render::NNRenderAssetManager;
using NN::Runtime::Render::NNTextureSourceAsset;

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_getImGuiTextureHandle(std::uint64_t textureKey)
{
	return NNRenderAssetManager::Get().GetImGuiTextureHandle(textureKey);
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_createTextureFromPixels(
	std::uint32_t width,
	std::uint32_t height,
	const std::uint8_t* pixels,
	std::size_t pixelSize,
	int isSRGB)
{
	return NNRenderAssetManager::Get().CreateTextureFromPixels(
		width, height, pixels, pixelSize, isSRGB != 0);
}

void NN_ENGINE_ABI_STDCALL rt_renderAsset_releaseTexture(std::uint64_t textureKey)
{
	NNRenderAssetManager::Get().ReleaseTexture(textureKey);
}

void NN_ENGINE_ABI_STDCALL rt_renderAsset_reloadTextureFromPixels(
	std::uint64_t textureKey,
	std::uint32_t width,
	std::uint32_t height,
	const std::uint8_t* pixels,
	std::size_t pixelSize,
	int isSRGB)
{
	/* 从原始像素构造 Source Asset，再调用 ReloadTexture */
	std::vector<std::uint8_t> pixelCopy(pixels, pixels + pixelSize);
	NNTextureSourceAsset source;
	source.SetFromDecodedImage(width, height,
		NN::Runtime::Render::NNTextureFormat::RGBA8_UNorm,
		std::move(pixelCopy), isSRGB != 0, true);
	NNRenderAssetManager::Get().ReloadTexture(textureKey, source);
}

int NN_ENGINE_ABI_STDCALL rt_renderAsset_getTextureDesc(
	std::uint64_t textureKey,
	std::uint32_t* outWidth,
	std::uint32_t* outHeight)
{
	auto* res = NNRenderAssetManager::Get().GetTextureResource(textureKey);
	if (!res)
		return 0;
	const auto& desc = res->GetDesc();
	if (outWidth)  *outWidth  = desc.Width;
	if (outHeight) *outHeight = desc.Height;
	return 1;
}

int NN_ENGINE_ABI_STDCALL rt_renderAsset_isTextureResident(std::uint64_t textureKey)
{
	auto* res = NNRenderAssetManager::Get().GetTextureResource(textureKey);
	return (res && res->IsResident()) ? 1 : 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_getCachedTextureCount(void)
{
	return static_cast<std::uint64_t>(NNRenderAssetManager::Get().GetCachedTextureCount());
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_getTotalGPUMemory(void)
{
	return static_cast<std::uint64_t>(NNRenderAssetManager::Get().GetEstimatedGPUMemory());
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_loadTextureFromAsset(std::uint64_t assetHandle, std::uint64_t guidLow)
{
	return NNRenderAssetManager::Get().LoadTextureFromAsset(assetHandle, guidLow);
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_renderAsset_loadTextureFromBlob(
	const void* typeInfoData,
	std::uint64_t typeInfoSize,
	const void* pixelData,
	std::uint64_t pixelDataSize,
	std::uint64_t guidLow)
{
	return NNRenderAssetManager::Get().LoadTextureFromBlob(
		typeInfoData, typeInfoSize, pixelData, pixelDataSize, guidLow);
}

} // namespace

extern "C" void NNBuildRenderAssetRuntimeApi(NNRenderAssetAPI* api)
{
	std::cout << "Building RenderAsset Runtime API..." << std::endl;
	if (api == nullptr)
	{
		return;
	}

	api->getImGuiTextureHandle  = &rt_renderAsset_getImGuiTextureHandle;
	api->createTextureFromPixels = &rt_renderAsset_createTextureFromPixels;
	api->releaseTexture          = &rt_renderAsset_releaseTexture;
	api->reloadTextureFromPixels = &rt_renderAsset_reloadTextureFromPixels;
	api->getTextureDesc          = &rt_renderAsset_getTextureDesc;
	api->isTextureResident       = &rt_renderAsset_isTextureResident;
	api->getCachedTextureCount   = &rt_renderAsset_getCachedTextureCount;
	api->getTotalGPUMemory       = &rt_renderAsset_getTotalGPUMemory;
	api->loadTextureFromAsset    = &rt_renderAsset_loadTextureFromAsset;
	api->loadTextureFromBlob     = &rt_renderAsset_loadTextureFromBlob;

	std::cout << "RenderAsset Runtime API built." << std::endl;
}
