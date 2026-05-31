/**
 * @file ViewportRenderApiStubs.cpp
 * @brief NNViewportRenderAPI 默认 Stub：返回 0，不执行实际渲染。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

namespace
{

std::uint64_t NN_ENGINE_ABI_STDCALL stub_viewportRender_renderSceneToTexture(
    std::uint64_t sceneHandle,
    std::uint32_t width,
    std::uint32_t height)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)sceneHandle;
    (void)width;
    (void)height;
    return 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_viewportRender_getLastRenderedTexture(void)
{
    NN::StubRuntime::BumpInvokeCount();
    return 0;
}

void NN_ENGINE_ABI_STDCALL stub_viewportRender_getRenderStats(
    std::uint32_t* outDrawCalls,
    std::uint32_t* outQuadCount)
{
    NN::StubRuntime::BumpInvokeCount();
    if (outDrawCalls) *outDrawCalls = 0;
    if (outQuadCount) *outQuadCount = 0;
}

void NN_ENGINE_ABI_STDCALL stub_viewportRender_setRmlUIViewportSize(
    std::uint32_t width,
    std::uint32_t height)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)width;
    (void)height;
}

void NN_ENGINE_ABI_STDCALL stub_viewportRender_processRmlUIInput(
    std::uint32_t type,
    std::int32_t mouseX, std::int32_t mouseY,
    std::int32_t wheelX, std::int32_t wheelY,
    std::uint32_t button,
    std::uint32_t keyCode, std::uint32_t keyMod)
{
    NN::StubRuntime::BumpInvokeCount();
    (void)type;
    (void)mouseX;
    (void)mouseY;
    (void)wheelX;
    (void)wheelY;
    (void)button;
    (void)keyCode;
    (void)keyMod;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_viewportRender_getLastRmluiTexture(void)
{
    NN::StubRuntime::BumpInvokeCount();
    return 0;
}

} // namespace

extern "C" void NNBuildViewportRenderApiStubs(NNViewportRenderAPI* api)
{
    if (api == nullptr)
    {
        return;
    }
    api->RenderSceneToTexture = &stub_viewportRender_renderSceneToTexture;
    api->GetLastRenderedTexture = &stub_viewportRender_getLastRenderedTexture;
    api->GetRenderStats = &stub_viewportRender_getRenderStats;
    api->SetRmlUIViewportSize = &stub_viewportRender_setRmlUIViewportSize;
    api->ProcessRmlUIInput = &stub_viewportRender_processRmlUIInput;
    api->GetLastRmluiTexture = &stub_viewportRender_getLastRmluiTexture;
}
