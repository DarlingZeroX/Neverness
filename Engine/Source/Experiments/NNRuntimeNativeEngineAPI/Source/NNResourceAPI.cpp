// NNResourceAPI.cpp -- C API for resource queries and updates implementation

#include "../API/NNResourceAPI.h"
#include "../API/NNEngineContext.h"
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>

using namespace NN::Runtime::Core;
using namespace NN::Runtime::Render;
using namespace NN::Runtime::NativeAPI;

extern "C" {

// ===== Buffer =====

uint32_t NNE_BufferGetSize(NNE_Handle buffer)
{
    auto* obj = static_cast<INNBuffer*>(NNEngineContext::Get().GetResource(buffer));
    if (!obj) return 0;
    return obj->GetSize();
}

void NNE_BufferUpdateData(NNE_Handle buffer, const void* data, uint32_t size, uint32_t offset)
{
    auto* obj = static_cast<INNBuffer*>(NNEngineContext::Get().GetResource(buffer));
    if (!obj || !data) return;
    obj->UpdateData(data, size, offset);
}

// ===== Texture =====

uint32_t NNE_TextureGetWidth(NNE_Handle texture)
{
    auto* obj = static_cast<INNTexture*>(NNEngineContext::Get().GetResource(texture));
    if (!obj) return 0;
    return obj->GetWidth();
}

uint32_t NNE_TextureGetHeight(NNE_Handle texture)
{
    auto* obj = static_cast<INNTexture*>(NNEngineContext::Get().GetResource(texture));
    if (!obj) return 0;
    return obj->GetHeight();
}

// ===== RenderTarget =====

uint32_t NNE_RenderTargetGetWidth(NNE_Handle rt)
{
    auto* obj = static_cast<INNRenderTarget*>(NNEngineContext::Get().GetResource(rt));
    if (!obj) return 0;
    return obj->GetWidth();
}

uint32_t NNE_RenderTargetGetHeight(NNE_Handle rt)
{
    auto* obj = static_cast<INNRenderTarget*>(NNEngineContext::Get().GetResource(rt));
    if (!obj) return 0;
    return obj->GetHeight();
}

} // extern "C"
