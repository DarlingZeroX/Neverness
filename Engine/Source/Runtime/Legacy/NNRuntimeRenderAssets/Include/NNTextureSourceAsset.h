/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNTextureFormat.h"
#include "../../NNRuntimeRenderAssets/NNRuntimeRenderAssetsExport.h"
#include <cstdint>
#include <vector>

namespace NN::Runtime::Render
{

/// Mip Level 数据
struct NNMipLevel
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    std::vector<uint8_t> Pixels;  // 行优先，紧密排列
};

/// 纹理源资产（CPU 侧）
/// 代表 import pipeline 产物
/// 纯 CPU，不允许 GPU API
class NN_RUNTIME_RENDER_ASSETS_API NNTextureSourceAsset
{
public:
    NNTextureSourceAsset() = default;

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetMipCount() const { return static_cast<uint32_t>(m_Mips.size()); }
    NNTextureFormat GetFormat() const { return m_Format; }
    bool IsSRGB() const { return m_IsSRGB; }
    bool HasAlpha() const { return m_HasAlpha; }

    const NNMipLevel& GetMip(uint32_t level) const { return m_Mips[level]; }
    const std::vector<NNMipLevel>& GetMips() const { return m_Mips; }

    /// 从解码图像初始化（base mip only）
    void SetFromDecodedImage(
        uint32_t width, uint32_t height,
        NNTextureFormat format,
        std::vector<uint8_t> pixels,
        bool isSRGB, bool hasAlpha)
    {
        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_IsSRGB = isSRGB;
        m_HasAlpha = hasAlpha;

        NNMipLevel mip;
        mip.Width = width;
        mip.Height = height;
        mip.Pixels = std::move(pixels);

        m_Mips.clear();
        m_Mips.push_back(std::move(mip));
    }

    /// 设置完整 mip 链
    void SetMips(std::vector<NNMipLevel> mips)
    {
        m_Mips = std::move(mips);
        if (!m_Mips.empty())
        {
            m_Width = m_Mips[0].Width;
            m_Height = m_Mips[0].Height;
        }
    }

    /// 序列化到二进制 blob（用于 .nnasset 存储）
    /// 格式: [Width:u32][Height:u32][Format:u32][Flags:u32][MipCount:u32][Mip0Size:u32][Mip0Pixels...][Mip1Size:u32][Mip1Pixels...]...
    std::vector<uint8_t> Serialize() const;

    /// 从二进制 blob 反序列化
    bool Deserialize(const uint8_t* data, size_t size);

    size_t GetTotalByteSize() const;

    bool IsValid() const { return m_Width > 0 && m_Height > 0 && !m_Mips.empty(); }

    void Reset()
    {
        m_Width = m_Height = 0;
        m_Format = NNTextureFormat::RGBA8_UNorm;
        m_IsSRGB = m_HasAlpha = false;
        m_Mips.clear();
    }

private:
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    NNTextureFormat m_Format = NNTextureFormat::RGBA8_UNorm;
    bool m_IsSRGB = false;
    bool m_HasAlpha = false;
    std::vector<NNMipLevel> m_Mips;
};

} // namespace NN::Runtime::Render
