/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNDecodedImage.h"
#include <string>

namespace NN::Runtime::ImageCodec
{

/// 纯函数式图像解码接口
/// 无状态，线程安全
namespace ImageCodecFunc
{
    /// 从内存解码图像
    /// @param data 压缩图像数据（PNG/JPG/TGA/BMP/HDR）
    /// @param dataSize 数据字节数
    /// @param desiredChannels 期望通道数（0=自动，3=RGB，4=RGBA）
    /// @return 解码结果；失败时 IsValid() == false
    NNDecodedImage DecodeFromMemory(
        const uint8_t* data,
        size_t dataSize,
        uint32_t desiredChannels = 0
    );

    /// 从文件路径解码
    NNDecodedImage DecodeFromFile(
        const std::string& filePath,
        uint32_t desiredChannels = 0
    );

    /// 生成 mipmap（CPU 侧，简单 box filter）
    /// @param source 源图像（必须是 RGBA8）
    /// @param mipLevels 目标 mip 层数（含 base level）
    /// @return 从 mip0 到 mipN 的像素数据，每个 mip 紧密排列
    std::vector<uint8_t> GenerateMipsCPU(
        const NNDecodedImage& source,
        uint32_t mipLevels
    );

    /// 判断格式是否为 HDR（float 数据）
    bool IsHDRFormat(NNImageFormat format);

    /// 计算给定宽高的最大 mip 层数
    uint32_t CalculateMipCount(uint32_t width, uint32_t height);

    /// 获取格式对应的每像素字节数
    uint32_t GetBytesPerPixel(NNImageFormat format);

} // namespace ImageCodecFunc

} // namespace NN::Runtime::ImageCodec
