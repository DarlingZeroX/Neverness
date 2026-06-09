/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNTextureResource.h"

#include "NNCore/Interface/HLog.h"

namespace NN::Runtime::Render
{

NNTextureResource::NNTextureResource() = default;

NNTextureResource::~NNTextureResource()
{
    // 不释放 GPU 资源 — 所有权在 NNRenderAssetManager 的 shared_ptr 中
}

NNTextureResource::NNTextureResource(NNTextureResource&& other) noexcept
    : m_Desc(other.m_Desc)
    , m_RHITexture(other.m_RHITexture)
    , m_RHIShaderResourceView(other.m_RHIShaderResourceView)
    , m_LastUsedFrame(other.m_LastUsedFrame)
    , m_Residency(other.m_Residency)
{
    other.m_RHITexture = nullptr;
    other.m_RHIShaderResourceView = nullptr;
    other.m_Residency = NNTextureResidency::NotLoaded;
}

NNTextureResource& NNTextureResource::operator=(NNTextureResource&& other) noexcept
{
    if (this != &other)
    {
        // 不释放旧 GPU 资源 — 所有权在 Manager
        m_Desc = other.m_Desc;
        m_RHITexture = other.m_RHITexture;
        m_RHIShaderResourceView = other.m_RHIShaderResourceView;
        m_LastUsedFrame = other.m_LastUsedFrame;
        m_Residency = other.m_Residency;

        other.m_RHITexture = nullptr;
        other.m_RHIShaderResourceView = nullptr;
        other.m_Residency = NNTextureResidency::NotLoaded;
    }
    return *this;
}

uint64_t NNTextureResource::GetImGuiHandle() const
{
    if (!m_RHIShaderResourceView)
        return 0;

    // 直接使用创建时缓存的 SRV 值
    // 由 IRenderResourceFactory 在创建纹理时填充
    // 后端无关：reinterpret_cast<uint64_t>(void* srv)
    uint64_t handle = reinterpret_cast<uint64_t>(m_RHIShaderResourceView);
    return handle;
}

void NNTextureResource::ReleaseGPU()
{
    // GPU 资源生命周期由 NNRenderAssetManager 的 shared_ptr 管理
    // 此处仅清除观察指针
    m_RHITexture = nullptr;
    m_RHIShaderResourceView = nullptr;
    m_Residency = NNTextureResidency::NotLoaded;
}

} // namespace NN::Runtime::Render
