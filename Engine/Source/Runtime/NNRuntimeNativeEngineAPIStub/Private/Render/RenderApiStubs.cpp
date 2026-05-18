/**
 * @file RenderApiStubs.cpp
 * @brief **NNRenderAPI** 預設 Stub：不建立真實紋理或渲染目標，僅遞增測試計數。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
NNTextureHandle NN_ENGINE_ABI_STDCALL stub_render_createTexture(std::uint32_t width, std::uint32_t height)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)width;
	(void)height;
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_render_uploadTexture(
	NNTextureHandle texture,
	const std::uint8_t* pixelBytes,
	std::size_t byteCount)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)texture;
	(void)pixelBytes;
	(void)byteCount;
}

NNRenderTargetHandle NN_ENGINE_ABI_STDCALL stub_render_createRenderTarget(std::uint32_t width, std::uint32_t height)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)width;
	(void)height;
	return 0;
}
} // namespace

extern "C" void NNBuildRenderApiStubs(NNRenderAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->createTexture = &stub_render_createTexture;
	api->uploadTexture = &stub_render_uploadTexture;
	api->createRenderTarget = &stub_render_createRenderTarget;
}
