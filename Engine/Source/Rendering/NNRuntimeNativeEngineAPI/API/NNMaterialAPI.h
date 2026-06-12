// NNMaterialAPI.h — C API for Material and SRP (C# interop)
// Exposes NNRuntimeRenderAssets and NNRuntimeSRP as handle-based C functions.

#pragma once

#include "NNRenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===== Shader Asset =====
// Load shader from HLSL source string
NNE_Handle NNE_LoadShaderFromMemory(NNE_Handle device, const char* source, uint32_t length,
                                     int stage, const char* name, const char* entryPoint);

// Load shader from file
NNE_Handle NNE_LoadShaderFromFile(NNE_Handle device, const char* path, int stage,
                                   const char* entryPoint);

// Release shader asset
void NNE_ReleaseShaderAsset(NNE_Handle shaderAsset);

// ===== Material =====
// Create empty material
NNE_Handle NNE_CreateMaterial(void);

// Set shaders on material
void NNE_MaterialSetVS(NNE_Handle material, NNE_Handle shaderAsset);
void NNE_MaterialSetPS(NNE_Handle material, NNE_Handle shaderAsset);

// Set material parameters
void NNE_MaterialSetFloat(NNE_Handle material, const char* name, float value);
void NNE_MaterialSetInt(NNE_Handle material, const char* name, int value);
void NNE_MaterialSetVec2(NNE_Handle material, const char* name, float x, float y);
void NNE_MaterialSetVec3(NNE_Handle material, const char* name, float x, float y, float z);
void NNE_MaterialSetVec4(NNE_Handle material, const char* name, float x, float y, float z, float w);
void NNE_MaterialSetTexture(NNE_Handle material, const char* name, NNE_Handle texture);
void NNE_MaterialSetSampler(NNE_Handle material, const char* name, NNE_Handle sampler);

// Set render state
void NNE_MaterialSetCullMode(NNE_Handle material, int cullMode);
void NNE_MaterialSetBlendEnable(NNE_Handle material, int enable);
void NNE_MaterialSetDepthEnable(NNE_Handle material, int depthEnable, int depthWrite);

// Get parameter
float NNE_MaterialGetFloat(NNE_Handle material, const char* name);
int   NNE_MaterialGetInt(NNE_Handle material, const char* name);

// Release material
void NNE_ReleaseMaterial(NNE_Handle material);

// ===== Material Instance =====
// Create instance from base material (shares shader, can override params)
NNE_Handle NNE_CreateMaterialInstance(NNE_Handle baseMaterial);

// Override parameters (same setters as material)
void NNE_MatInstSetFloat(NNE_Handle instance, const char* name, float value);
void NNE_MatInstSetInt(NNE_Handle instance, const char* name, int value);
void NNE_MatInstSetVec4(NNE_Handle instance, const char* name, float x, float y, float z, float w);
void NNE_MatInstSetTexture(NNE_Handle instance, const char* name, NNE_Handle texture);
void NNE_MatInstSetSampler(NNE_Handle instance, const char* name, NNE_Handle sampler);

// Release instance
void NNE_ReleaseMaterialInstance(NNE_Handle instance);

// ===== SRP Pipeline =====
// Create forward pipeline
NNE_Handle NNE_CreateForwardPipeline(NNE_Handle device, uint32_t width, uint32_t height);

// Destroy pipeline
void NNE_DestroyPipeline(NNE_Handle pipeline);

// Resize pipeline (recreate render targets)
void NNE_ResizePipeline(NNE_Handle pipeline, uint32_t width, uint32_t height);

// Get pipeline pass count
uint32_t NNE_PipelineGetPassCount(NNE_Handle pipeline);

// ===== Render Context (data struct for C# marshalling) =====
#pragma pack(push, 1)
typedef struct NNE_Vec3 {
    float x, y, z;
} NNE_Vec3;

typedef struct NNE_Mat4x4 {
    float m[16];
} NNE_Mat4x4;

typedef struct NNE_CameraData {
    NNE_Mat4x4 ViewMatrix;
    NNE_Mat4x4 ProjMatrix;
    NNE_Mat4x4 ViewProjMatrix;
    NNE_Vec3 Position;
    float NearPlane;
    float FarPlane;
    float FOV;
    float AspectRatio;
} NNE_CameraData;

typedef struct NNE_LightData {
    int Type;           // 0=Directional, 1=Point, 2=Spot
    NNE_Vec3 Direction;
    NNE_Vec3 Position;
    NNE_Vec3 Color;
    float Intensity;
    int CastShadows;
    float ShadowBias;
    NNE_Mat4x4 LightViewProj;
} NNE_LightData;

typedef struct NNE_RenderContextData {
    NNE_CameraData Camera;
    NNE_LightData Lights[16];
    int LightCount;
    float DeltaTime;
    float TotalTime;
    uint64_t FrameNumber;
    uint32_t FrameWidth;
    uint32_t FrameHeight;
} NNE_RenderContextData;
#pragma pack(pop)

// Execute pipeline with render context
void NNE_ExecutePipeline(NNE_Handle pipeline, NNE_Handle cmdList, const NNE_RenderContextData* ctx);

#ifdef __cplusplus
}
#endif
