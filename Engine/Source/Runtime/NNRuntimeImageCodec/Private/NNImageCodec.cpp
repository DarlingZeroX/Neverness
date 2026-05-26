/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNImageCodec.h"

// stb_image 声明 — 实现在 stb_image_impl.cpp 中
#include <NNCore/Include/Stb/stb_image.h>

#include <algorithm>
#include <cmath>

namespace NN::Runtime::ImageCodec
{

// 从 stb 返回的 channels 映射到 NNImageFormat
static NNImageFormat ChannelsToFormat(uint32_t channels, bool isHDR)
{
    if (isHDR)
    {
        switch (channels)
        {
        case 1: return NNImageFormat::R32F;
        case 2: return NNImageFormat::RG32F;
        case 3: return NNImageFormat::RGB32F;
        case 4: return NNImageFormat::RGBA32F;
        default: return NNImageFormat::Unknown;
        }
    }
    else
    {
        switch (channels)
        {
        case 1: return NNImageFormat::R8;
        case 2: return NNImageFormat::RG8;
        case 3: return NNImageFormat::RGB8;
        case 4: return NNImageFormat::RGBA8;
        default: return NNImageFormat::Unknown;
        }
    }
}

NNDecodedImage ImageCodecFunc::DecodeFromMemory(
    const uint8_t* data,
    size_t dataSize,
    uint32_t desiredChannels)
{
    NNDecodedImage result;

    if (!data || dataSize == 0)
        return result;

    // 检测是否为 HDR
    int isHDR = stbi_is_hdr_from_memory(
        reinterpret_cast<const stbi_uc*>(data),
        static_cast<int>(dataSize)
    );

    int w = 0, h = 0, ch = 0;

    if (isHDR)
    {
        float* pixels = stbi_loadf_from_memory(
            reinterpret_cast<const stbi_uc*>(data),
            static_cast<int>(dataSize),
            &w, &h, &ch,
            static_cast<int>(desiredChannels)
        );

        if (!pixels)
            return result;

        uint32_t channels = desiredChannels > 0 ? desiredChannels : static_cast<uint32_t>(ch);
        result.Width = static_cast<uint32_t>(w);
        result.Height = static_cast<uint32_t>(h);
        result.Channels = channels;
        result.Format = ChannelsToFormat(channels, true);

        size_t byteCount = static_cast<size_t>(w) * h * channels * sizeof(float);
        result.Pixels.resize(byteCount);
        std::memcpy(result.Pixels.data(), pixels, byteCount);

        stbi_image_free(pixels);
    }
    else
    {
        uint8_t* pixels = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(data),
            static_cast<int>(dataSize),
            &w, &h, &ch,
            static_cast<int>(desiredChannels)
        );

        if (!pixels)
            return result;

        uint32_t channels = desiredChannels > 0 ? desiredChannels : static_cast<uint32_t>(ch);
        result.Width = static_cast<uint32_t>(w);
        result.Height = static_cast<uint32_t>(h);
        result.Channels = channels;
        result.Format = ChannelsToFormat(channels, false);

        size_t byteCount = static_cast<size_t>(w) * h * channels;
        result.Pixels.resize(byteCount);
        std::memcpy(result.Pixels.data(), pixels, byteCount);

        stbi_image_free(pixels);
    }

    return result;
}

NNDecodedImage ImageCodecFunc::DecodeFromFile(
    const std::string& filePath,
    uint32_t desiredChannels)
{
    NNDecodedImage result;

    if (filePath.empty())
        return result;

    int isHDR = stbi_is_hdr(filePath.c_str());

    int w = 0, h = 0, ch = 0;

    if (isHDR)
    {
        float* pixels = stbi_loadf(
            filePath.c_str(),
            &w, &h, &ch,
            static_cast<int>(desiredChannels)
        );

        if (!pixels)
            return result;

        uint32_t channels = desiredChannels > 0 ? desiredChannels : static_cast<uint32_t>(ch);
        result.Width = static_cast<uint32_t>(w);
        result.Height = static_cast<uint32_t>(h);
        result.Channels = channels;
        result.Format = ChannelsToFormat(channels, true);

        size_t byteCount = static_cast<size_t>(w) * h * channels * sizeof(float);
        result.Pixels.resize(byteCount);
        std::memcpy(result.Pixels.data(), pixels, byteCount);

        stbi_image_free(pixels);
    }
    else
    {
        uint8_t* pixels = stbi_load(
            filePath.c_str(),
            &w, &h, &ch,
            static_cast<int>(desiredChannels)
        );

        if (!pixels)
            return result;

        uint32_t channels = desiredChannels > 0 ? desiredChannels : static_cast<uint32_t>(ch);
        result.Width = static_cast<uint32_t>(w);
        result.Height = static_cast<uint32_t>(h);
        result.Channels = channels;
        result.Format = ChannelsToFormat(channels, false);

        size_t byteCount = static_cast<size_t>(w) * h * channels;
        result.Pixels.resize(byteCount);
        std::memcpy(result.Pixels.data(), pixels, byteCount);

        stbi_image_free(pixels);
    }

    return result;
}

std::vector<uint8_t> ImageCodecFunc::GenerateMipsCPU(
    const NNDecodedImage& source,
    uint32_t mipLevels)
{
    std::vector<uint8_t> result;

    if (!source.IsValid() || source.Format != NNImageFormat::RGBA8 || mipLevels == 0)
        return result;

    const uint32_t channels = 4; // RGBA8
    const uint32_t baseW = source.Width;
    const uint32_t baseH = source.Height;

    // 预计算总大小
    size_t totalSize = 0;
    uint32_t w = baseW, h = baseH;
    for (uint32_t i = 0; i < mipLevels && w > 0 && h > 0; ++i)
    {
        totalSize += static_cast<size_t>(w) * h * channels;
        w = std::max(1u, w / 2);
        h = std::max(1u, h / 2);
    }

    result.resize(totalSize);
    uint8_t* dst = result.data();

    // mip0: 复制源数据
    size_t mip0Size = static_cast<size_t>(baseW) * baseH * channels;
    std::memcpy(dst, source.Pixels.data(), mip0Size);
    dst += mip0Size;

    // 后续 mip: 简单 2x2 box filter
    const uint8_t* prevMip = source.Pixels.data();
    uint32_t prevW = baseW, prevH = baseH;

    for (uint32_t mip = 1; mip < mipLevels && prevW > 1 && prevH > 1; ++mip)
    {
        uint32_t newW = std::max(1u, prevW / 2);
        uint32_t newH = std::max(1u, prevH / 2);

        for (uint32_t y = 0; y < newH; ++y)
        {
            for (uint32_t x = 0; x < newW; ++x)
            {
                uint32_t srcX0 = x * 2;
                uint32_t srcY0 = y * 2;
                uint32_t srcX1 = std::min(srcX0 + 1, prevW - 1);
                uint32_t srcY1 = std::min(srcY0 + 1, prevH - 1);

                for (uint32_t c = 0; c < channels; ++c)
                {
                    uint32_t a = prevMip[(srcY0 * prevW + srcX0) * channels + c];
                    uint32_t b = prevMip[(srcY0 * prevW + srcX1) * channels + c];
                    uint32_t cc = prevMip[(srcY1 * prevW + srcX0) * channels + c];
                    uint32_t d = prevMip[(srcY1 * prevW + srcX1) * channels + c];

                    dst[(y * newW + x) * channels + c] =
                        static_cast<uint8_t>((a + b + cc + d + 2) / 4);
                }
            }
        }

        prevMip = dst;
        dst += static_cast<size_t>(newW) * newH * channels;
        prevW = newW;
        prevH = newH;
    }

    return result;
}

bool ImageCodecFunc::IsHDRFormat(NNImageFormat format)
{
    switch (format)
    {
    case NNImageFormat::R16F:
    case NNImageFormat::RG16F:
    case NNImageFormat::RGB16F:
    case NNImageFormat::RGBA16F:
    case NNImageFormat::R32F:
    case NNImageFormat::RG32F:
    case NNImageFormat::RGB32F:
    case NNImageFormat::RGBA32F:
        return true;
    default:
        return false;
    }
}

uint32_t ImageCodecFunc::CalculateMipCount(uint32_t width, uint32_t height)
{
    uint32_t count = 1;
    while (width > 1 || height > 1)
    {
        width = std::max(1u, width / 2);
        height = std::max(1u, height / 2);
        ++count;
    }
    return count;
}

uint32_t ImageCodecFunc::GetBytesPerPixel(NNImageFormat format)
{
    switch (format)
    {
    case NNImageFormat::R8:     return 1;
    case NNImageFormat::RG8:    return 2;
    case NNImageFormat::RGB8:   return 3;
    case NNImageFormat::RGBA8:  return 4;
    case NNImageFormat::R16F:   return 2;
    case NNImageFormat::RG16F:  return 4;
    case NNImageFormat::RGB16F: return 6;
    case NNImageFormat::RGBA16F:return 8;
    case NNImageFormat::R32F:   return 4;
    case NNImageFormat::RG32F:  return 8;
    case NNImageFormat::RGB32F: return 12;
    case NNImageFormat::RGBA32F:return 16;
    default: return 0;
    }
}

} // namespace NN::Runtime::ImageCodec
