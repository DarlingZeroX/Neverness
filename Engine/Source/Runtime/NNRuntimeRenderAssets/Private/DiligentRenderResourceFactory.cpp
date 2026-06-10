/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "DiligentRenderResourceFactory.h"

// INNRenderDevice + INNTexture（定义 Experiments 版 NNTextureDesc）
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/Resources/INNTexture.h>

#include <NNCore/Interface/HLog.h>

namespace NN::Runtime::Render
{
    // 格式映射：NNTextureFormat → NNPixelFormat（Experiments 版）
    static NNPixelFormat ToPixelFormat(NNTextureFormat format, bool isSRGB)
    {
        switch (format)
        {
        case NNTextureFormat::RGBA8_UNorm:  return isSRGB ? NNPixelFormat::RGBA8_SRGB : NNPixelFormat::RGBA8_UNORM;
        case NNTextureFormat::R32_Float:    return NNPixelFormat::R32_FLOAT;
        case NNTextureFormat::RG32_Float:   return NNPixelFormat::RG32_FLOAT;
        case NNTextureFormat::RGBA32_Float: return NNPixelFormat::RGBA32_FLOAT;
        case NNTextureFormat::BC1_UNorm:    return NNPixelFormat::BC1_UNORM;
        case NNTextureFormat::BC3_UNorm:    return NNPixelFormat::BC3_UNORM;
        case NNTextureFormat::BC7_UNorm:    return NNPixelFormat::BC7_UNORM;
        default:                            return NNPixelFormat::RGBA8_UNORM;
        }
    }

    DiligentRenderResourceFactory::DiligentRenderResourceFactory(INNRenderDevice* device)
        : m_Device(device)
    {
    }

    std::unique_ptr<NNTextureResource> DiligentRenderResourceFactory::CreateTexture(
        uint32_t width, uint32_t height,
        NNTextureFormat format,
        const uint8_t* pixels, size_t pixelSize)
    {
        if (!m_Device || !pixels || pixelSize == 0 || width == 0 || height == 0)
        {
            H_LOG_ERROR("[DiligentRenderResourceFactory] CreateTexture: 参数无效");
            return nullptr;
        }

        // 构造 Experiments 版 NNTextureDesc（用于 INNRenderDevice::CreateTexture）
        NNTextureDesc texDesc{};
        texDesc.Width     = width;
        texDesc.Height    = height;
        texDesc.MipLevels = 1;
        texDesc.Format    = ToPixelFormat(format, false);
        texDesc.Usage     = NNTextureUsage::Default;
        texDesc.DebugName = "NNRenderAsset_Texture";

        // 通过 INNRenderDevice 创建纹理
        auto texture = m_Device->CreateTexture(texDesc, pixels);
        if (!texture)
        {
            H_LOG_ERROR("[DiligentRenderResourceFactory] CreateTexture: 设备创建失败 %ux%u", width, height);
            return nullptr;
        }

        // 获取 SRV（通过接口方法，不依赖 Diligent 类型）
        void* srv = texture->GetShaderResourceView();
        if (!srv)
        {
            H_LOG_ERROR("[DiligentRenderResourceFactory] CreateTexture: 获取 SRV 失败");
            return nullptr;
        }

        // 构造 NNTextureResource（Runtime 版，m_Desc 类型为 NNTextureCacheDesc）
        auto resource = std::make_unique<NNTextureResource>();
        resource->m_Desc.Width    = width;
        resource->m_Desc.Height   = height;
        resource->m_Desc.MipCount = 1;
        resource->m_Desc.Format   = format;
        resource->m_Desc.IsSRGB   = false;
        resource->m_RHITexture            = texture.Detach();  // 转移所有权到 void*
        resource->m_RHIShaderResourceView = srv;
        resource->m_Residency = NNTextureResidency::Resident;

        return resource;
    }

    bool DiligentRenderResourceFactory::UpdateTexturePixels(
        NNTextureResource* resource,
        const uint8_t* pixels, size_t pixelSize)
    {
        // TODO: 需要 IDeviceContext* 来调用 UpdateTexture
        H_LOG_WARN("[DiligentRenderResourceFactory] UpdateTexturePixels: 暂未实现");
        return false;
    }

} // namespace NN::Runtime::Render
