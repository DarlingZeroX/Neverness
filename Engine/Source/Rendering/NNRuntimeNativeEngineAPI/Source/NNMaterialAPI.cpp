// NNMaterialAPI.cpp — C API implementation for Material and SRP

#include "../API/NNMaterialAPI.h"
#include "../API/NNEngineContext.h"
#include <NNRuntimeRenderAssets/Material/NNMaterial.h>
#include <NNRuntimeRenderAssets/Material/NNMaterialInstance.h>
#include <NNRuntimeRenderAssets/Shader/NNShaderAsset.h>
#include <NNRuntimeRenderAssets/Cache/NNPipelineCache.h>
#include <NNRuntimeSRP/Pipeline/NNForwardPipeline.h>
#include <NNRuntimeSRP/Context/NNRenderContext.h>
#include <iostream>
#include <unordered_map>

using namespace NN::Runtime::Core;
using namespace NN::Runtime::Render;
using namespace NN::Runtime::Assets;
using namespace NN::Runtime::NativeAPI;
using namespace NN::Runtime::SRP;

extern "C" {

// ==============================================================================
//  Local caches (separate from handle registry for type safety)
// ============================================================================

static std::unordered_map<NNE_Handle, NNShaderAsset*> g_ShaderAssetCache;
static std::unordered_map<NNE_Handle, NNMaterial*> g_MaterialCache;
static std::unordered_map<NNE_Handle, NNMaterialInstance*> g_MaterialInstanceCache;
static std::unordered_map<NNE_Handle, NNForwardPipeline*> g_PipelineCache;
static std::unordered_map<NNE_Handle, NNPipelineCache*> g_PipelineCacheObj;

// Handle counter for assets (separate from GPU resource handles)
static uint64_t g_NextAssetHandle = 0x100000000ULL; // Start above GPU handles

static NNE_Handle AllocAssetHandle()
{
    return g_NextAssetHandle++;
}

// ============================================================================
//  Shader Asset
// ============================================================================

NNE_Handle NNE_LoadShaderFromMemory(NNE_Handle device, const char* source, uint32_t length,
                                     int stage, const char* name, const char* entryPoint)
{
    auto* dev = static_cast<INNRenderDevice*>(NNEngineContext::Get().GetResource(device));
    if (!dev || !source) return NNE_INVALID_HANDLE;

    auto asset = NNShaderAsset::LoadFromMemory(source, length,
                                                 static_cast<NNShaderStage>(stage),
                                                 name, entryPoint);
    if (!asset) return NNE_INVALID_HANDLE;

    // Create shader immediately
    auto shader = asset->CreateShader(dev);
    if (!shader) return NNE_INVALID_HANDLE;

    // Register shader in handle registry
    NNE_Handle handle = NNEngineContext::Get().RegisterResource(NNHandleType::Shader, shader.Get());
    if (handle != NNE_INVALID_HANDLE)
    {
        g_ShaderAssetCache[handle] = asset.Get();
        // Keep asset alive
        asset->AddRef();
    }
    return handle;
}

NNE_Handle NNE_LoadShaderFromFile(NNE_Handle device, const char* path, int stage,
                                   const char* entryPoint)
{
    auto* dev = static_cast<INNRenderDevice*>(NNEngineContext::Get().GetResource(device));
    if (!dev || !path) return NNE_INVALID_HANDLE;

    auto asset = NNShaderAsset::LoadFromFile(path, static_cast<NNShaderStage>(stage), entryPoint);
    if (!asset) return NNE_INVALID_HANDLE;

    auto shader = asset->CreateShader(dev);
    if (!shader) return NNE_INVALID_HANDLE;

    NNE_Handle handle = NNEngineContext::Get().RegisterResource(NNHandleType::Shader, shader.Get());
    if (handle != NNE_INVALID_HANDLE)
    {
        g_ShaderAssetCache[handle] = asset.Get();
        asset->AddRef();
    }
    return handle;
}

void NNE_ReleaseShaderAsset(NNE_Handle shaderAsset)
{
    auto it = g_ShaderAssetCache.find(shaderAsset);
    if (it != g_ShaderAssetCache.end())
    {
        it->second->Release();
        g_ShaderAssetCache.erase(it);
    }
    NNEngineContext::Get().ReleaseResource(shaderAsset);
}

// ============================================================================
//  Material
// ============================================================================

NNE_Handle NNE_CreateMaterial(void)
{
    auto* mat = new NNMaterial();
    NNE_Handle handle = AllocAssetHandle();
    g_MaterialCache[handle] = mat;
    mat->AddRef();
    return handle;
}

void NNE_MaterialSetVS(NNE_Handle material, NNE_Handle shaderAsset)
{
    auto matIt = g_MaterialCache.find(material);
    if (matIt == g_MaterialCache.end()) return;

    // Get shader from handle registry
    auto* shader = static_cast<INNShader*>(NNEngineContext::Get().GetResource(shaderAsset));
    if (shader)
    {
        matIt->second->SetVertexShader(NNRef<INNShader>(shader));
    }
}

void NNE_MaterialSetPS(NNE_Handle material, NNE_Handle shaderAsset)
{
    auto matIt = g_MaterialCache.find(material);
    if (matIt == g_MaterialCache.end()) return;

    auto* shader = static_cast<INNShader*>(NNEngineContext::Get().GetResource(shaderAsset));
    if (shader)
    {
        matIt->second->SetPixelShader(NNRef<INNShader>(shader));
    }
}

void NNE_MaterialSetFloat(NNE_Handle material, const char* name, float value)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end()) it->second->SetFloat(name, value);
}

void NNE_MaterialSetInt(NNE_Handle material, const char* name, int value)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end()) it->second->SetInt(name, value);
}

void NNE_MaterialSetVec2(NNE_Handle material, const char* name, float x, float y)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end()) it->second->SetVector2(name, x, y);
}

void NNE_MaterialSetVec3(NNE_Handle material, const char* name, float x, float y, float z)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end()) it->second->SetVector3(name, x, y, z);
}

void NNE_MaterialSetVec4(NNE_Handle material, const char* name, float x, float y, float z, float w)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end()) it->second->SetVector4(name, x, y, z, w);
}

void NNE_MaterialSetTexture(NNE_Handle material, const char* name, NNE_Handle texture)
{
    auto it = g_MaterialCache.find(material);
    if (it == g_MaterialCache.end()) return;

    auto* tex = static_cast<INNTexture*>(NNEngineContext::Get().GetResource(texture));
    if (tex)
    {
        it->second->SetTexture(name, NNRef<INNTexture>(tex));
    }
}

void NNE_MaterialSetSampler(NNE_Handle material, const char* name, NNE_Handle sampler)
{
    auto it = g_MaterialCache.find(material);
    if (it == g_MaterialCache.end()) return;

    auto* s = static_cast<INNSampler*>(NNEngineContext::Get().GetResource(sampler));
    if (s)
    {
        it->second->SetSampler(name, NNRef<INNSampler>(s));
    }
}

void NNE_MaterialSetCullMode(NNE_Handle material, int cullMode)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end())
    {
        it->second->SetCullMode(static_cast<NNCullMode>(cullMode));
    }
}

void NNE_MaterialSetBlendEnable(NNE_Handle material, int enable)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end())
    {
        it->second->SetBlendEnable(enable != 0);
    }
}

void NNE_MaterialSetDepthEnable(NNE_Handle material, int depthEnable, int depthWrite)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end())
    {
        it->second->SetDepthEnable(depthEnable != 0);
        it->second->SetDepthWrite(depthWrite != 0);
    }
}

float NNE_MaterialGetFloat(NNE_Handle material, const char* name)
{
    auto it = g_MaterialCache.find(material);
    if (it == g_MaterialCache.end()) return 0.0f;

    const auto* p = it->second->GetParam(name);
    if (p && p->Type == NNMaterialParamType::Float) return p->Float;
    return 0.0f;
}

int NNE_MaterialGetInt(NNE_Handle material, const char* name)
{
    auto it = g_MaterialCache.find(material);
    if (it == g_MaterialCache.end()) return 0;

    const auto* p = it->second->GetParam(name);
    if (p && p->Type == NNMaterialParamType::Int) return p->Int;
    return 0;
}

void NNE_ReleaseMaterial(NNE_Handle material)
{
    auto it = g_MaterialCache.find(material);
    if (it != g_MaterialCache.end())
    {
        it->second->Release();
        g_MaterialCache.erase(it);
    }
}

// ============================================================================
//  Material Instance
// ============================================================================

NNE_Handle NNE_CreateMaterialInstance(NNE_Handle baseMaterial)
{
    auto baseIt = g_MaterialCache.find(baseMaterial);
    if (baseIt == g_MaterialCache.end()) return NNE_INVALID_HANDLE;

    // Create NNRef from raw pointer (AddRef inside)
    NNRef<NNMaterial> baseRef(baseIt->second);
    auto* inst = new NNMaterialInstance(baseRef);

    NNE_Handle handle = AllocAssetHandle();
    g_MaterialInstanceCache[handle] = inst;
    inst->AddRef();
    return handle;
}

void NNE_MatInstSetFloat(NNE_Handle instance, const char* name, float value)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it != g_MaterialInstanceCache.end()) it->second->SetFloat(name, value);
}

void NNE_MatInstSetInt(NNE_Handle instance, const char* name, int value)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it != g_MaterialInstanceCache.end()) it->second->SetInt(name, value);
}

void NNE_MatInstSetVec4(NNE_Handle instance, const char* name, float x, float y, float z, float w)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it != g_MaterialInstanceCache.end()) it->second->SetVector4(name, x, y, z, w);
}

void NNE_MatInstSetTexture(NNE_Handle instance, const char* name, NNE_Handle texture)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it == g_MaterialInstanceCache.end()) return;

    auto* tex = static_cast<INNTexture*>(NNEngineContext::Get().GetResource(texture));
    if (tex)
    {
        it->second->SetTexture(name, NNRef<INNTexture>(tex));
    }
}

void NNE_MatInstSetSampler(NNE_Handle instance, const char* name, NNE_Handle sampler)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it == g_MaterialInstanceCache.end()) return;

    auto* s = static_cast<INNSampler*>(NNEngineContext::Get().GetResource(sampler));
    if (s)
    {
        it->second->SetSampler(name, NNRef<INNSampler>(s));
    }
}

void NNE_ReleaseMaterialInstance(NNE_Handle instance)
{
    auto it = g_MaterialInstanceCache.find(instance);
    if (it != g_MaterialInstanceCache.end())
    {
        it->second->Release();
        g_MaterialInstanceCache.erase(it);
    }
}

// ============================================================================
//  SRP Pipeline
// ============================================================================

NNE_Handle NNE_CreateForwardPipeline(NNE_Handle device, uint32_t width, uint32_t height)
{
    auto* dev = static_cast<INNRenderDevice*>(NNEngineContext::Get().GetResource(device));
    if (!dev) return NNE_INVALID_HANDLE;

    auto* pipeline = new NNForwardPipeline();
    if (!pipeline->Initialize(dev, width, height))
    {
        delete pipeline;
        return NNE_INVALID_HANDLE;
    }

    NNE_Handle handle = AllocAssetHandle();
    g_PipelineCache[handle] = pipeline;
    pipeline->AddRef();

    // Also create a pipeline cache for materials
    auto* pCache = new NNPipelineCache(dev);
    pCache->AddRef();
    g_PipelineCacheObj[handle] = pCache;

    return handle;
}

void NNE_DestroyPipeline(NNE_Handle pipeline)
{
    auto it = g_PipelineCache.find(pipeline);
    if (it != g_PipelineCache.end())
    {
        it->second->Shutdown();
        it->second->Release();
        g_PipelineCache.erase(it);
    }
    auto pIt = g_PipelineCacheObj.find(pipeline);
    if (pIt != g_PipelineCacheObj.end())
    {
        pIt->second->Release();
        g_PipelineCacheObj.erase(pIt);
    }
}

void NNE_ResizePipeline(NNE_Handle pipeline, uint32_t width, uint32_t height)
{
    auto it = g_PipelineCache.find(pipeline);
    if (it != g_PipelineCache.end())
    {
        it->second->OnResize(width, height);
    }
}

uint32_t NNE_PipelineGetPassCount(NNE_Handle pipeline)
{
    auto it = g_PipelineCache.find(pipeline);
    if (it != g_PipelineCache.end())
    {
        return it->second->GetPassCount();
    }
    return 0;
}

void NNE_ExecutePipeline(NNE_Handle pipeline, NNE_Handle cmdList, const NNE_RenderContextData* ctx)
{
    auto pIt = g_PipelineCache.find(pipeline);
    if (pIt == g_PipelineCache.end()) return;

    auto* cmd = static_cast<INNCommandList*>(NNEngineContext::Get().GetResource(cmdList));
    if (!cmd || !ctx) return;

    // Convert C# data to C++ context
    NNRenderContext renderCtx;
    renderCtx.Device = NNEngineContext::Get().GetDevice();
    renderCtx.PipelineCache = nullptr;

    // Find pipeline cache
    auto pcIt = g_PipelineCacheObj.find(pipeline);
    if (pcIt != g_PipelineCacheObj.end())
    {
        renderCtx.PipelineCache = pcIt->second;
    }

    renderCtx.FrameWidth = ctx->FrameWidth;
    renderCtx.FrameHeight = ctx->FrameHeight;
    renderCtx.DeltaTime = ctx->DeltaTime;
    renderCtx.TotalTime = ctx->TotalTime;
    renderCtx.FrameNumber = ctx->FrameNumber;

    // Camera
    renderCtx.Camera.Position = {ctx->Camera.Position.x, ctx->Camera.Position.y, ctx->Camera.Position.z};
    renderCtx.Camera.NearPlane = ctx->Camera.NearPlane;
    renderCtx.Camera.FarPlane = ctx->Camera.FarPlane;
    renderCtx.Camera.FOV = ctx->Camera.FOV;
    renderCtx.Camera.AspectRatio = ctx->Camera.AspectRatio;

    // Copy matrices
    std::memcpy(renderCtx.Camera.ViewMatrix.m, ctx->Camera.ViewMatrix.m, sizeof(float) * 16);
    std::memcpy(renderCtx.Camera.ProjMatrix.m, ctx->Camera.ProjMatrix.m, sizeof(float) * 16);
    std::memcpy(renderCtx.Camera.ViewProjMatrix.m, ctx->Camera.ViewProjMatrix.m, sizeof(float) * 16);

    // Lights
    int lightCount = ctx->LightCount;
    if (lightCount > 16) lightCount = 16;
    for (int i = 0; i < lightCount; ++i)
    {
        NNLightData light;
        light.Type = static_cast<NNLightType>(ctx->Lights[i].Type);
        light.Direction = {ctx->Lights[i].Direction.x, ctx->Lights[i].Direction.y, ctx->Lights[i].Direction.z};
        light.Position = {ctx->Lights[i].Position.x, ctx->Lights[i].Position.y, ctx->Lights[i].Position.z};
        light.Color = {ctx->Lights[i].Color.x, ctx->Lights[i].Color.y, ctx->Lights[i].Color.z};
        light.Intensity = ctx->Lights[i].Intensity;
        light.CastShadows = ctx->Lights[i].CastShadows != 0;
        light.ShadowBias = ctx->Lights[i].ShadowBias;
        std::memcpy(light.LightViewProj.m, ctx->Lights[i].LightViewProj.m, sizeof(float) * 16);
        renderCtx.Scene.Lights.push_back(light);
    }

    // Execute
    pIt->second->Execute(cmd, renderCtx);
}

} // extern "C"
