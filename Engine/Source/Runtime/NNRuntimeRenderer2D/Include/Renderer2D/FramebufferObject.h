#pragma once

/**
 * @file FramebufferObject.h
 * @brief 离屏渲染目标封装，对外只暴露 TextureHandle（供 ImGui.Image 使用）。
 *
 * 内部使用 Diligent INNRenderTarget 实现。
 * 对外不暴露 Diligent 类型，使用 uint64_t 作为纹理句柄。
 */

#include "Renderer2DExport.h"
#include <cstdint>

// 前向声明
namespace NN::Runtime::Render
{
    class INNRenderDevice;
    class INNRenderTarget;
}

namespace NN::Runtime::Renderer2D
{
    /// 离屏渲染目标封装
    class NN_RUNTIME_RENDERER2D_API FramebufferObject
    {
    public:
        FramebufferObject() = default;
        ~FramebufferObject();

        FramebufferObject(const FramebufferObject&) = delete;
        FramebufferObject& operator=(const FramebufferObject&) = delete;

        /// 创建渲染目标
        /// @param device Diligent 渲染设备
        bool Initialize(Render::INNRenderDevice* device, std::uint32_t width, std::uint32_t height);
        void Shutdown();

        /// 重设尺寸（窗口 resize 时调用）
        void Resize(std::uint32_t width, std::uint32_t height);

        /// 获取颜色附件的纹理句柄（供 ImGui.Image 使用）
        /// 返回 reinterpret_cast<uint64_t>(ITextureView* SRV)
        std::uint64_t GetColorTextureHandle() const;

        /// 获取颜色附件 RTV 指针（供 Renderer2D BeginRenderPass 使用）
        void* GetColorRTV() const;

        /// 获取深度模板附件 DSV 指针（供 Renderer2D BeginRenderPass 使用）
        void* GetDepthDSV() const;

        /// 获取内部渲染目标（供 Renderer2D 绑定使用）
        Render::INNRenderTarget* GetRenderTarget() const { return m_RenderTarget; }

        std::uint32_t GetWidth() const { return m_Width; }
        std::uint32_t GetHeight() const { return m_Height; }

    private:
        Render::INNRenderDevice* m_Device = nullptr;     // 观察指针，不持有所有权
        Render::INNRenderTarget* m_RenderTarget = nullptr; // 渲染目标（持有所有权，通过 NNRef）
        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;
    };
}
