/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include <cstdint>
#include <vector>

namespace NN::Runtime::ImageCodec
{

/// 纹素格式（引擎级，不依赖任何 GPU API）
enum class NNImageFormat : uint32_t
{
    Unknown = 0,
    R8,
    RG8,
    RGB8,
    RGBA8,
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
};

/// 解码后的 CPU 图像数据
/// 纯 CPU，不允许任何 GPU API
struct NNDecodedImage
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t Channels = 0;
    NNImageFormat Format = NNImageFormat::Unknown;
    std::vector<uint8_t> Pixels;   // 行优先，紧密排列，无 padding

    bool IsValid() const { return !Pixels.empty() && Width > 0 && Height > 0; }
    size_t GetByteSize() const { return Pixels.size(); }

    void Reset()
    {
        Width = Height = Channels = 0;
        Format = NNImageFormat::Unknown;
        Pixels.clear();
    }
};

} // namespace NN::Runtime::ImageCodec
