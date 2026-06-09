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
        Shutdown();
        Initialize(m_Device, width, height);
    }

    std::uint64_t FramebufferObject::GetColorTextureHandle() const
    {
        if (!m_RenderTarget)
            return 0;

        // 获取 Diligent 渲染目标的颜色纹理 SRV
        auto* dilRT = static_cast<NNDiligent::NNDiligentRenderTarget*>(
            const_cast<Render::INNRenderTarget*>(m_RenderTarget));
        auto* colorView = dilRT->GetColorView();
        if (!colorView)
            return 0;

        // 返回 ITextureView* 作为 uint64_t
        return reinterpret_cast<std::uint64_t>(colorView);
    }

    FramebufferObject::~FramebufferObject()
    {
        Shutdown();
    }
}
