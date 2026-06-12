// NNResourceAPI.h -- C API for resource queries and updates
// Provides property getters and data update functions for GPU resources.

#pragma once

#include "NNRenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===== Buffer =====
uint32_t NNE_BufferGetSize(NNE_Handle buffer);
void NNE_BufferUpdateData(NNE_Handle buffer, const void* data, uint32_t size, uint32_t offset);

// ===== Texture =====
uint32_t NNE_TextureGetWidth(NNE_Handle texture);
uint32_t NNE_TextureGetHeight(NNE_Handle texture);

// ===== RenderTarget =====
uint32_t NNE_RenderTargetGetWidth(NNE_Handle rt);
uint32_t NNE_RenderTargetGetHeight(NNE_Handle rt);

#ifdef __cplusplus
}
#endif
