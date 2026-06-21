/**
 * @file FramebufferObject.cpp
 * @brief 离屏渲染目标实现（Diligent INNRenderTarget）。
 *
 * 使用 INNRenderDevice 创建渲染目标，通过 Diligent ITextureView 获取纹理句柄。
 */

#include "Renderer2D/FramebufferObject.h"

// NNRuntimeRender 接口
#include <Device/INNRenderDevice.h>
#include <RenderTarget/INNRenderTarget.h>

// NNDiligent 内部（用于获取 Diligent 对象）
#include "NNDiligentConfig.h"
#include "Device/NNDiligentDevice.h"
#include "NNCore/Interface/HLog.h"
#include "Resources/NNDiligentRenderTarget.h"

using namespace Diligent;

namespace NN::Runtime::Renderer2D
{
    bool FramebufferObject::Initialize(Render::INNRenderDevice* device, std::uint32_t width, std::uint32_t height)
    {
        if (!device || width == 0 || height == 0)
            return false;

        m_Device = device;
        m_Width  = width;
        m_Height = height;

        // 通过 INNRenderDevice 创建渲染目标
        Render::NNRenderTargetDesc rtDesc{};
        rtDesc.Width       = width;
        rtDesc.Height      = height;
        // UNORM 格式：sRGB 转换在 Sprite shader 中手动处理（pow 1/2.2）。
        // 不能用 SRGB：RmlUI 整个管线在 sRGB 空间工作，FBO 必须是 UNORM。
        rtDesc.ColorFormat = Render::NNPixelFormat::RGBA8_UNORM;
        rtDesc.DepthFormat = Render::NNPixelFormat::D24_UNORM_S8_UINT;
        rtDesc.SampleCount = 1;

        auto rt = device->CreateRenderTarget(rtDesc);
        if (!rt)
            return false;

        m_RenderTarget = rt.Detach(); // 转移所有权
        return true;
    }

    void FramebufferObject::Shutdown()
    {
        if (m_RenderTarget)
        {
            m_RenderTarget->Release();
            m_RenderTarget = nullptr;
        }
        m_Device = nullptr;
        m_Width  = 0;
        m_Height = 0;
    }

    void FramebufferObject::Resize(std::uint32_t width, std::uint32_t height)
    {
        if (width == m_Width && height == m_Height)
            return;
        if (width == 0 || height == 0)
            return;
        // 保存 device 指针，Shutdown 会清空它
        auto* device = m_Device;
        Shutdown();
        Initialize(device, width, height);
    }

    std::uint64_t FramebufferObject::GetColorTextureHandle() const
    {
        if (!m_RenderTarget)
        {
            static bool s_logged1 = false;
            if (!s_logged1) { H_LOG_WARN("[FBO] GetColorTextureHandle: m_RenderTarget is null"); s_logged1 = true; }
            return 0;
        }

        // 直接获取 SRV（在 NNDiligentRenderTarget 初始化时创建）
        auto* dilRT = static_cast<NNDiligent::NNDiligentRenderTarget*>(
            const_cast<Render::INNRenderTarget*>(m_RenderTarget));
        auto* srv = dilRT->GetColorSRV();
        if (!srv)
        {
            H_LOG_WARN("[FBO] GetColorTextureHandle: srv is null");
            return 0;
        }

        // 返回 SRV 指针作为 uint64_t
        return reinterpret_cast<std::uint64_t>(srv);
    }

    void* FramebufferObject::GetColorRTV() const
    {
        if (!m_RenderTarget) return nullptr;
        auto* dilRT = static_cast<NNDiligent::NNDiligentRenderTarget*>(
            const_cast<Render::INNRenderTarget*>(m_RenderTarget));
        return dilRT->GetColorView();
    }

    void* FramebufferObject::GetDepthDSV() const
    {
        if (!m_RenderTarget) return nullptr;
        auto* dilRT = static_cast<NNDiligent::NNDiligentRenderTarget*>(
            const_cast<Render::INNRenderTarget*>(m_RenderTarget));
        return dilRT->GetDepthView();
    }

    FramebufferObject::~FramebufferObject()
    {
        Shutdown();
    }
}
