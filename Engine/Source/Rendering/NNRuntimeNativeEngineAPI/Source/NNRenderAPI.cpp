// NNRenderAPI.cpp -- C API for rendering operations implementation

#include "../API/NNRenderAPI.h"
#include "../API/NNEngineContext.h"
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>
#include <iostream>

using namespace NN::Runtime::Core;
using namespace NN::Runtime::Render;
using namespace NN::Runtime::NativeAPI;

static NNE_Device* g_Device = nullptr;

extern "C" {

// ===== Device =====
NNE_Device* NNE_GetDevice(void) { return g_Device; }
void NNE_SetDevice(NNE_Device* device) { g_Device = device; }

// ===== Resource Creation =====

NNE_Handle NNE_CreateBuffer(uint32_t size, uint32_t type, uint32_t usage, const void* initialData)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNBufferDesc desc{};
    desc.Type = static_cast<NNBufferType>(type);
    desc.Usage = static_cast<NNBufferUsage>(usage);
    desc.Size = size;

    auto buf = ctx->CreateBuffer(desc, initialData);
    if (!buf) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Buffer, buf.Get());
}

NNE_Handle NNE_CreateTexture(uint32_t width, uint32_t height, uint32_t format, uint32_t usage, const char* debugName)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNTextureDesc desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = static_cast<NNPixelFormat>(format);
    desc.Usage = static_cast<NNTextureUsage>(usage);
    desc.DebugName = debugName;

    auto tex = ctx->CreateTexture(desc);
    if (!tex) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Texture, tex.Get());
}

NNE_Handle NNE_CreateSampler(uint32_t minFilter, uint32_t magFilter, uint32_t addressU, uint32_t addressV)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNSamplerDesc desc{};
    desc.MinFilter = static_cast<NNFilterMode>(minFilter);
    desc.MagFilter = static_cast<NNFilterMode>(magFilter);
    desc.AddressU = static_cast<NNAddressMode>(addressU);
    desc.AddressV = static_cast<NNAddressMode>(addressV);

    auto sampler = ctx->CreateSampler(desc);
    if (!sampler) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Sampler, sampler.Get());
}

// Static shader cache to avoid handle registry casting issues
static std::unordered_map<NNE_Handle, INNShader*> g_ShaderCache;

NNE_Handle NNE_CreateShader(uint32_t stage, const char* sourceCode, const char* entryPoint, const char* debugName)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNShaderDesc desc{};
    desc.Stage = static_cast<NNShaderStage>(stage);
    desc.SourceCode = sourceCode;
    desc.EntryPoint = entryPoint;
    desc.DebugName = debugName;

    auto shader = ctx->CreateShader(desc);
    if (!shader) return NNE_INVALID_HANDLE;

    INNShader* rawPtr = shader.Get();
    NNE_Handle handle = NNEngineContext::Get().RegisterResource(NNHandleType::Shader, rawPtr);
    if (handle != NNE_INVALID_HANDLE)
    {
        g_ShaderCache[handle] = rawPtr;
    }
    return handle;
}

NNE_Handle NNE_CreatePipelineState(NNE_Handle vs, NNE_Handle ps, const void* psoDescPtr)
{
    auto* device = NNEngineContext::Get().GetDevice();
    if (!device) return NNE_INVALID_HANDLE;

    // Retrieve shaders from cache (bypasses handle registry casting issues)
    auto vsIt = g_ShaderCache.find(vs);
    auto psIt = g_ShaderCache.find(ps);
    if (vsIt == g_ShaderCache.end() || psIt == g_ShaderCache.end()) return NNE_INVALID_HANDLE;

    auto* vsObj = vsIt->second;
    auto* psObj = psIt->second;
    if (!vsObj || !psObj) return NNE_INVALID_HANDLE;

    // Use provided desc or default
    NNPipelineStateDesc psoDesc{};
    if (psoDescPtr)
    {
        psoDesc = *static_cast<const NNPipelineStateDesc*>(psoDescPtr);
    }
    psoDesc.VS = vsObj;
    psoDesc.PS = psObj;

    auto pso = device->CreatePipelineState(psoDesc);
    if (!pso) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Pipeline, pso.Get());
}

NNE_Handle NNE_CreateRenderTarget(uint32_t width, uint32_t height, uint32_t colorFormat, uint32_t depthFormat)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNRenderTargetDesc desc{};
    desc.Width = width;
    desc.Height = height;
    desc.ColorFormat = static_cast<NNPixelFormat>(colorFormat);
    desc.DepthFormat = static_cast<NNPixelFormat>(depthFormat);

    auto rt = ctx->CreateRenderTarget(desc);
    if (!rt) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::RenderTarget, rt.Get());
}

// ===== Resource Destruction =====

void NNE_ReleaseHandle(NNE_Handle handle)
{
    NNEngineContext::Get().ReleaseResource(handle);
}

// ===== Query =====

uint32_t NNE_GetHandleType(NNE_Handle handle)
{
    return static_cast<uint32_t>(GetHandleType(handle));
}

uint32_t NNE_IsHandleValid(NNE_Handle handle)
{
    return IsHandleValid(handle) ? 1u : 0u;
}

// ===== Render Commands =====

void NNE_ClearRenderTarget(float r, float g, float b, float a)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = static_cast<NNDiligent::NNDiligentCommandList*>(dev->GetImmediateCommandList());
    if (cmd) cmd->ClearRenderTarget(r, g, b, a);
}

void NNE_SetPipelineState(NNE_Handle pso)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    auto* psoObj = static_cast<INNPipelineState*>(NNEngineContext::Get().GetResource(pso));
    if (cmd && psoObj) cmd->SetPipelineState(psoObj);
}

void NNE_SetVertexBuffer(NNE_Handle vb, uint32_t slot)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    auto* bufObj = static_cast<INNBuffer*>(NNEngineContext::Get().GetResource(vb));
    if (cmd && bufObj) cmd->SetVertexBuffer(bufObj, slot);
}

void NNE_SetIndexBuffer(NNE_Handle ib)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    auto* bufObj = static_cast<INNBuffer*>(NNEngineContext::Get().GetResource(ib));
    if (cmd && bufObj) cmd->SetIndexBuffer(bufObj);
}

void NNE_SetViewport(float x, float y, float w, float h, float minD, float maxD)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    if (!cmd) return;

    NNViewport vp{};
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    vp.Width = w;
    vp.Height = h;
    vp.MinDepth = minD;
    vp.MaxDepth = maxD;
    cmd->SetViewports(&vp, 1);
}

void NNE_Draw(uint32_t vertexCount, uint32_t startVertex, uint32_t instanceCount)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    if (!cmd) return;

    NNDrawAttribs da{};
    da.VertexCount = vertexCount;
    da.StartVertexLocation = startVertex;
    da.InstanceCount = instanceCount;
    cmd->Draw(da);
}

void NNE_DrawIndexed(uint32_t indexCount, uint32_t startIndex, int32_t baseVertex, uint32_t instanceCount)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    if (!cmd) return;

    NNDrawIndexedAttribs da{};
    da.IndexCount = indexCount;
    da.StartIndexLocation = startIndex;
    da.BaseVertexLocation = baseVertex;
    da.InstanceCount = instanceCount;
    cmd->DrawIndexed(da);
}

void NNE_Present(void)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = static_cast<NNDiligent::NNDiligentCommandList*>(dev->GetImmediateCommandList());
    if (cmd) cmd->Present();
}

} // extern "C"
