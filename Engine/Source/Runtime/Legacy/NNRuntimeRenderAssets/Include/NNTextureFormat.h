/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include <cstdint>

namespace NN::Runtime::Render
{

/// 引擎级纹理格式
/// 不映射任何 API 特定格式
/// RHI 后端负责映射到 OpenGL/Diligent/Vulkan 格式
enum class NNTextureFormat : uint32_t
{
    Unknown = 0,

    // 8-bit unorm
    R8_UNorm,
    RG8_UNorm,
    RGB8_UNorm,
    RGBA8_UNorm,

    // 16-bit float
    R16_Float,
    RG16_Float,
    RGB16_Float,
    RGBA16_Float,

    // 32-bit float
    R32_Float,
    RG32_Float,
    RGB32_Float,
    RGBA32_Float,

    // 压缩格式（预留）
    BC1_UNorm,
    BC3_UNorm,
    BC7_UNorm,
    ETC2_RGB8,
    ETC2_RGBA8,
    ASTC_4x4,

    Count
};

/// 获取每个纹素的字节数（未压缩格式）
inline uint32_t GetBytesPerPixel(NNTextureFormat format)
{
    switch (format)
    {
    case NNTextureFormat::R8_UNorm:     return 1;
    case NNTextureFormat::RG8_UNorm:    return 2;
    case NNTextureFormat::RGB8_UNorm:   return 3;
    case NNTextureFormat::RGBA8_UNorm:  return 4;
    case NNTextureFormat::R16_Float:    return 2;
    case NNTextureFormat::RG16_Float:   return 4;
    case NNTextureFormat::RGB16_Float:  return 6;
    case NNTextureFormat::RGBA16_Float: return 8;
    case NNTextureFormat::R32_Float:    return 4;
    case NNTextureFormat::RG32_Float:   return 8;
    case NNTextureFormat::RGB32_Float:  return 12;
    case NNTextureFormat::RGBA32_Float: return 16;
    default: return 0;
    }
}

} // namespace NN::Runtime::Render
