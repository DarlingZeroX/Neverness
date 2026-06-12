// NNRenderAPI.h -- C API for rendering operations
// Exposes NNRuntimeRender interfaces as handle-based functions for C# interop.

#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Handle type (same as NNRenderHandle but exposed to C)
typedef uint64_t NNE_Handle;
#define NNE_INVALID_HANDLE 0

// Forward declarations
typedef struct NNE_Device NNE_Device;
typedef struct NNE_Context NNE_Context;

// ===== Device =====
NNE_Device* NNE_GetDevice(void);
void NNE_SetDevice(NNE_Device* device);

// ===== Resource Creation (returns Handle) =====
NNE_Handle NNE_CreateBuffer(uint32_t size, uint32_t type, uint32_t usage, const void* initialData);
NNE_Handle NNE_CreateTexture(uint32_t width, uint32_t height, uint32_t format, uint32_t usage, const char* debugName);
NNE_Handle NNE_CreateSampler(uint32_t minFilter, uint32_t magFilter, uint32_t addressU, uint32_t addressV);
NNE_Handle NNE_CreateShader(uint32_t stage, const char* sourceCode, const char* entryPoint, const char* debugName);
NNE_Handle NNE_CreatePipelineState(NNE_Handle vs, NNE_Handle ps, const void* psoDesc);
NNE_Handle NNE_CreateRenderTarget(uint32_t width, uint32_t height, uint32_t colorFormat, uint32_t depthFormat);

// ===== Resource Destruction =====
void NNE_ReleaseHandle(NNE_Handle handle);

// ===== Query =====
uint32_t NNE_GetHandleType(NNE_Handle handle);
uint32_t NNE_IsHandleValid(NNE_Handle handle);

// ===== Render Commands =====
void NNE_ClearRenderTarget(float r, float g, float b, float a);
void NNE_SetPipelineState(NNE_Handle pso);
void NNE_SetVertexBuffer(NNE_Handle vb, uint32_t slot);
void NNE_SetIndexBuffer(NNE_Handle ib);
void NNE_SetViewport(float x, float y, float w, float h, float minD, float maxD);
void NNE_Draw(uint32_t vertexCount, uint32_t startVertex, uint32_t instanceCount);
void NNE_DrawIndexed(uint32_t indexCount, uint32_t startIndex, int32_t baseVertex, uint32_t instanceCount);
void NNE_Present(void);

#ifdef __cplusplus
}
#endif
