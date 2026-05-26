/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNTextureResource.h"
#include "NNRuntimeRHI/Interface/Texture.h"

namespace NN::Runtime::VGFX
{
	struct ITexture;
}

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
    if (!m_RHITexture)
        return 0;

    // OpenGL 后端: ITexture* 实际是 Texture2D*，GetShaderResourceView() 返回 GLuint as void*
    // ImGui 的 OpenGL backend 的 ImTextureID 就是 GLuint
    // 未来 Diligent 后端: GetShaderResourceView() 返回 ITextureView* → ImTextureID
    auto* tex = static_cast<VGFX::ITexture*>(m_RHITexture);
    return reinterpret_cast<uint64_t>(tex->GetShaderResourceView());
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
