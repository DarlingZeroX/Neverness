#pragma once

/**
 * @file FramebufferObject.h
 * @brief 封装 OpenGL Framebuffer，对外只暴露 TextureId（供 ImGui.Image 使用）。
 *
 * 内部直接调用 OpenGL（MVP 阶段），对外不暴露 GLuint 类型。
 * 未来迁移 Diligent 时，此类内部实现替换为 Diligent ITextureView。
 */

#include "Renderer2DExport.h"
#include <cstdint>

namespace NN::Runtime::Renderer2D
{
    /// 离屏 Framebuffer 封装
    class NN_RUNTIME_RENDERER2D_API FramebufferObject
    {
    public:
        FramebufferObject() = default;
        ~FramebufferObject();

        FramebufferObject(const FramebufferObject&) = delete;
        FramebufferObject& operator=(const FramebufferObject&) = delete;

        /// 创建 Framebuffer
        bool Initialize(std::uint32_t width, std::uint32_t height);
        void Shutdown();

        /// 重设尺寸（窗口 resize 时调用）
        void Resize(std::uint32_t width, std::uint32_t height);

        /// 绑定此 FBO 为渲染目标
        void Bind();
        /// 解绑（恢复默认 Framebuffer）
        void Unbind();

        /// 获取颜色附件的 OpenGL Texture ID（供 ImGui.Image 使用）
        std::uint32_t GetColorTextureId() const { return m_ColorTextureId; }

        std::uint32_t GetWidth() const { return m_Width; }
        std::uint32_t GetHeight() const { return m_Height; }

    private:
        std::uint32_t m_FBO = 0;
        std::uint32_t m_ColorTextureId = 0;
        std::uint32_t m_DepthRenderbuffer = 0;
        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;
    };
}
