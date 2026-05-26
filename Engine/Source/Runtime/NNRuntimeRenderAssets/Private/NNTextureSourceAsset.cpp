/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNTextureSourceAsset.h"
#include <cstring>

namespace NN::Runtime::Render
{

// 序列化格式：
// [0  ] uint32 Width
// [4  ] uint32 Height
// [8  ] uint32 Format (NNTextureFormat)
// [12 ] uint32 Flags (bit0=sRGB, bit1=HasAlpha)
// [16 ] uint32 MipCount
// [20 ] uint32 Mip0Size, uint32 Mip1Size, ...
// [20+MipCount*4] Mip0Pixels..., Mip1Pixels...

std::vector<uint8_t> NNTextureSourceAsset::Serialize() const
{
    std::vector<uint8_t> result;

    if (!IsValid())
        return result;

    // 计算头大小
    const uint32_t headerSize = 20 + static_cast<uint32_t>(m_Mips.size()) * 4;
    const size_t totalSize = headerSize + GetTotalByteSize();

    result.resize(totalSize);
    uint8_t* dst = result.data();

    // 写入头部
    uint32_t flags = 0;
    if (m_IsSRGB) flags |= 1;
    if (m_HasAlpha) flags |= 2;

    std::memcpy(dst + 0, &m_Width, 4);
    std::memcpy(dst + 4, &m_Height, 4);
    uint32_t fmt = static_cast<uint32_t>(m_Format);
    std::memcpy(dst + 8, &fmt, 4);
    std::memcpy(dst + 12, &flags, 4);
    uint32_t mipCount = static_cast<uint32_t>(m_Mips.size());
    std::memcpy(dst + 16, &mipCount, 4);

    // 写入各 mip 大小
    uint8_t* sizePtr = dst + 20;
    for (const auto& mip : m_Mips)
    {
        uint32_t mipSize = static_cast<uint32_t>(mip.Pixels.size());
        std::memcpy(sizePtr, &mipSize, 4);
        sizePtr += 4;
    }

    // 写入各 mip 像素
    uint8_t* pixelPtr = dst + headerSize;
    for (const auto& mip : m_Mips)
    {
        std::memcpy(pixelPtr, mip.Pixels.data(), mip.Pixels.size());
        pixelPtr += mip.Pixels.size();
    }

    return result;
}

bool NNTextureSourceAsset::Deserialize(const uint8_t* data, size_t size)
{
    if (!data || size < 20)
        return false;

    Reset();

    const uint8_t* src = data;

    std::memcpy(&m_Width, src + 0, 4);
    std::memcpy(&m_Height, src + 4, 4);

    uint32_t fmt = 0;
    std::memcpy(&fmt, src + 8, 4);
    m_Format = static_cast<NNTextureFormat>(fmt);

    uint32_t flags = 0;
    std::memcpy(&flags, src + 12, 4);
    m_IsSRGB = (flags & 1) != 0;
    m_HasAlpha = (flags & 2) != 0;

    uint32_t mipCount = 0;
    std::memcpy(&mipCount, src + 16, 4);

    const uint32_t headerSize = 20 + mipCount * 4;
    if (size < headerSize)
        return false;

    // 读取各 mip 大小
    std::vector<uint32_t> mipSizes(mipCount);
    for (uint32_t i = 0; i < mipCount; ++i)
    {
        std::memcpy(&mipSizes[i], src + 20 + i * 4, 4);
    }

    // 读取各 mip 像素
    m_Mips.resize(mipCount);
    const uint8_t* pixelPtr = src + headerSize;

    for (uint32_t i = 0; i < mipCount; ++i)
    {
        if (pixelPtr + mipSizes[i] > src + size)
        {
            Reset();
            return false;
        }

        // 计算此 mip 的宽高
        m_Mips[i].Width = std::max(1u, m_Width >> i);
        m_Mips[i].Height = std::max(1u, m_Height >> i);
        m_Mips[i].Pixels.assign(pixelPtr, pixelPtr + mipSizes[i]);
        pixelPtr += mipSizes[i];
    }

    return true;
}

size_t NNTextureSourceAsset::GetTotalByteSize() const
{
    size_t total = 0;
    for (const auto& mip : m_Mips)
        total += mip.Pixels.size();
    return total;
}

} // namespace NN::Runtime::Render
