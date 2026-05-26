/**
 * @file RenderAssetApiStubs.cpp
 * @brief **NNRenderAssetAPI** 預設 Stub：不建立真實 GPU 紋理，僅遞增測試計數。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

namespace
{

std::uint64_t NN_ENGINE_ABI_STDCALL stub_renderAsset_getImGuiTextureHandle(std::uint64_t key)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)key;
    return 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_renderAsset_createTextureFromPixels(
    std::uint32_t width,
    std::uint32_t height,
    const std::uint8_t* pixels,
    std::size_t pixelSize,
    int isSRGB)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)width;
    (void)height;
    (void)pixels;
    (void)pixelSize;
    (void)isSRGB;
    return 0;
}

void NN_ENGINE_ABI_STDCALL stub_renderAsset_releaseTexture(std::uint64_t key)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)key;
}

void NN_ENGINE_ABI_STDCALL stub_renderAsset_reloadTextureFromPixels(
    std::uint64_t key,
    std::uint32_t width,
    std::uint32_t height,
    const std::uint8_t* pixels,
    std::size_t pixelSize,
    int isSRGB)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)key;
    (void)width;
    (void)height;
    (void)pixels;
    (void)pixelSize;
    (void)isSRGB;
}

int NN_ENGINE_ABI_STDCALL stub_renderAsset_getTextureDesc(
    std::uint64_t key,
    std::uint32_t* outWidth,
    std::uint32_t* outHeight)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)key;
    (void)outWidth;
    (void)outHeight;
    return 0;
}

int NN_ENGINE_ABI_STDCALL stub_renderAsset_isTextureResident(std::uint64_t key)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)key;
    return 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_renderAsset_getCachedTextureCount(void)
{
    NN::StubRuntime::BumpInvokeCount();
    return 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_renderAsset_getTotalGPUMemory(void)
{
    NN::StubRuntime::BumpInvokeCount();
    return 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_renderAsset_loadTextureFromAsset(std::uint64_t assetHandle)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)assetHandle;
    return 0;
}

} // namespace

extern "C" void NNBuildRenderAssetApiStubs(NNRenderAssetAPI* api)
{
    if (api == nullptr)
    {
        return;
    }
    api->getImGuiTextureHandle = &stub_renderAsset_getImGuiTextureHandle;
    api->createTextureFromPixels = &stub_renderAsset_createTextureFromPixels;
    api->releaseTexture = &stub_renderAsset_releaseTexture;
    api->reloadTextureFromPixels = &stub_renderAsset_reloadTextureFromPixels;
    api->getTextureDesc = &stub_renderAsset_getTextureDesc;
    api->isTextureResident = &stub_renderAsset_isTextureResident;
    api->getCachedTextureCount = &stub_renderAsset_getCachedTextureCount;
    api->getTotalGPUMemory = &stub_renderAsset_getTotalGPUMemory;
    api->loadTextureFromAsset = &stub_renderAsset_loadTextureFromAsset;
}
