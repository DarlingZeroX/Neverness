// =============================================================================
// RmlDiligentProgramId.h
// 着色器程序标识 — 用于 SRB 缓存键，对齐 GL3 Backend 的 ProgramId 划分
// =============================================================================
#pragma once

#include <cstdint>

namespace RmlDiligent {

/// RmlDiligent 内部 PSO/着色器变体 ID。同一纹理可能用于多种 PSO，故 SRB 按 ProgramId 分桶缓存。
enum class ProgramId : uint8_t {
    Invalid = 0,
    Color,
    Texture,
    TextureStencilEqual,  // m_PSO_Texture_StencilEqual 专用（implicit signature 不同）
    Gradient,
    Creation,
    Passthrough,
    PassthroughPresent,
    PassthroughOpacity,
    PassthroughReplace,
    Blur,
    DropShadow,
    ColorMatrix,
    BlendMask,
    Count
};

} // namespace RmlDiligent
