#pragma once
// =============================================================================
// RmlDiligentConfig.h
// RmlDiligent 模块配置头文件
// =============================================================================

#include <cstdint>
#include <cstddef>

// Diligent Core 头文件
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Texture.h"
#include "TextureView.h"
#include "PipelineState.h"
#include "Shader.h"
#include "ShaderResourceBinding.h"
#include "MapHelper.h"

// RmlDiligent 命名空间
namespace RmlDiligent {

// 句柄类型（不暴露 Diligent 对象给 RmlUi）
using CompiledGeometryHandle = uintptr_t;
using TextureHandle = uintptr_t;
using FilterHandle = uintptr_t;
using ShaderHandle = uintptr_t;
using LayerHandle = uintptr_t;

constexpr CompiledGeometryHandle InvalidGeometryHandle = 0;
constexpr TextureHandle InvalidTextureHandle = 0;
constexpr FilterHandle InvalidFilterHandle = 0;
constexpr ShaderHandle InvalidShaderHandle = 0;
constexpr LayerHandle InvalidLayerHandle = 0;

// ProgramId — PSO 变体枚举
enum class ProgramId : uint32_t {
    None = 0,

    // Color + Stencil 变体
    Color_Stencil_Always,
    Color_Stencil_Equal,
    Color_Stencil_Set,
    Color_Stencil_SetInverse,
    Color_Stencil_Intersect,
    Color_Stencil_Disabled,

    // Texture + Stencil 变体
    Texture_Stencil_Always,
    Texture_Stencil_Equal,
    Texture_Stencil_Disabled,

    // Gradient / Creation
    Gradient,
    Creation,

    // Passthrough 变体
    Passthrough,
    Passthrough_NoDepthStencil,
    Passthrough_Opacity,

    // Filter
    ColorMatrix,
    BlendMask,
    Blur,
    DropShadow,

    Count  // 总数
};

// 顶点格式（与 RmlUi::Vertex 一致）
struct RmlVertex {
    float position[2];   // 2D 位置
    uint8_t color[4];    // 预乘 alpha 颜色（RGBA）
    float tex_coord[2];  // 纹理坐标
};
static_assert(sizeof(RmlVertex) == 20, "RmlVertex size must be 20 bytes");

// Constant Buffer 布局（与 DX12 Backend 保持一致）

// 主 CB: VS_Main / PS_Texture / PS_Color
struct MainCB {
    float transform[16];  // 4x4 矩阵
    float translate[2];   // 平移偏移
    float padding[2];     // 对齐到 72 bytes
};
static_assert(sizeof(MainCB) == 72, "MainCB size must be 72 bytes");

// Blur CB: VS_Blur / PS_Blur
struct BlurCB {
    float transform[16];    // 64 bytes
    float translate[2];     // 8 bytes
    float texelOffset[2];   // 8 bytes
    float weights[4];       // 16 bytes
    float texCoordMin[2];   // 8 bytes
    float texCoordMax[2];   // 8 bytes
};
static_assert(sizeof(BlurCB) == 112, "BlurCB size must be 112 bytes");

// DropShadow CB: PS_DropShadow
struct DropShadowCB {
    float texCoordMin[2];   // 8 bytes
    float texCoordMax[2];   // 8 bytes
    float color[4];         // 16 bytes
};
static_assert(sizeof(DropShadowCB) == 32, "DropShadowCB size must be 32 bytes");

// ColorMatrix CB: PS_ColorMatrix
struct ColorMatrixCB {
    float colorMatrix[16];  // 4x4 矩阵
};
static_assert(sizeof(ColorMatrixCB) == 64, "ColorMatrixCB size must be 64 bytes");

// Gradient CB: PS_Gradient
struct GradientCB {
    float transform[16];           // 64 bytes
    float translate[2];            // 8 bytes
    float padding1[2];             // 8 bytes (对齐)
    int func;                      // 4 bytes
    int numStops;                  // 4 bytes
    float p[2];                    // 8 bytes
    float v[2];                    // 8 bytes
    float padding2[2];             // 8 bytes (对齐)
    float stopColors[16][4];       // 256 bytes
    float stopPositions[4][4];     // 64 bytes
};
static_assert(sizeof(GradientCB) == 416, "GradientCB size must be 416 bytes");

// Creation CB: PS_Creation
struct CreationCB {
    float transform[16];   // 64 bytes
    float translate[2];    // 8 bytes
    float dimensions[2];   // 8 bytes
    float value;           // 4 bytes
    float padding[3];      // 12 bytes (对齐到 96 bytes)
};
static_assert(sizeof(CreationCB) == 96, "CreationCB size must be 96 bytes");

} // namespace RmlDiligent
