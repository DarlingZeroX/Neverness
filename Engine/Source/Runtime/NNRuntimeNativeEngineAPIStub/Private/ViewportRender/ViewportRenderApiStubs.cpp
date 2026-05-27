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
}
