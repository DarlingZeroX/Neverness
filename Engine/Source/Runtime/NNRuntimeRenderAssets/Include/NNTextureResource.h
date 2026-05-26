/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNTextureFormat.h"
#include <cstdint>

namespace NN::Runtime::Render
{

/// GPU Texture 描述
struct NNTextureDesc
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t MipCount = 1;
    NNTextureFormat Format = NNTextureFormat::RGBA8_UNorm;
    bool IsSRGB = false;
};

/// Residency 状态（Streaming 预留）
enum class NNTextureResidency : uint8_t
{
    NotLoaded = 0,
    Loading,
    Resident,
    Evicted,
    Error,
};

/// GPU 纹理资源
/// 封装 RHI Texture，不暴露原生 Handle 类型
class NNTextureResource
{
public:
    NNTextureResource();
    ~NNTextureResource();

    // 禁止拷贝
    NNTextureResource(const NNTextureResource&) = delete;
    NNTextureResource& operator=(const NNTextureResource&) = delete;

    // 允许移动
    NNTextureResource(NNTextureResource&& other) noexcept;
    NNTextureResource& operator=(NNTextureResource&& other) noexcept;

    const NNTextureDesc& GetDesc() const { return m_Desc; }
    NNTextureResidency GetResidency() const { return m_Residency; }
    uint64_t GetLastUsedFrame() const { return m_LastUsedFrame; }

    /// 获取内部 RHI Texture 指针（仅 RenderAssetManager 内部使用）
    void* GetRHITextureInternal() const { return m_RHITexture; }

    /// 获取 ImGui 兼容的 Texture Handle
    /// OpenGL: GLuint cast to uint64
    /// Diligent: ITextureView* reinterpret to uint64
    uint64_t GetImGuiHandle() const;

    bool IsResident() const { return m_Residency == NNTextureResidency::Resident; }

    /// 释放 GPU 资源
    void ReleaseGPU();

private:
    friend class NNRenderAssetManager;

    NNTextureDesc m_Desc;
    void* m_RHITexture = nullptr;              // VGFX::ITexture*
    void* m_RHIShaderResourceView = nullptr;   // SRV（Diligent 后端用）
    uint64_t m_LastUsedFrame = 0;
    NNTextureResidency m_Residency = NNTextureResidency::NotLoaded;
};

} // namespace NN::Runtime::Render
