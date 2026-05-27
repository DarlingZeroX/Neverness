/**
 * @file FramebufferObject.cpp
 * @brief 离屏 Framebuffer 实现（直接调用 OpenGL）。
 */

#include "Renderer2D/FramebufferObject.h"
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>
#include <NNRuntimeRHI/Include/OpenGL/IncludeGladGL3.h>

namespace NN::Runtime::Renderer2D
{
    bool FramebufferObject::Initialize(std::uint32_t width, std::uint32_t height)
    {
        if (width == 0 || height == 0)
            return false;

        m_Width = width;
        m_Height = height;

        // 创建 FBO
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // 颜色附件：Texture2D
        glGenTextures(1, &m_ColorTextureId);
        glBindTexture(GL_TEXTURE_2D, m_ColorTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     static_cast<GLsizei>(width), static_cast<GLsizei>(height),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_ColorTextureId, 0);

        // 深度/模板附件：Renderbuffer
        glGenRenderbuffers(1, &m_DepthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, m_DepthRenderbuffer);

        // 完整性检查
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void FramebufferObject::Shutdown()
    {
        if (m_DepthRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &m_DepthRenderbuffer);
            m_DepthRenderbuffer = 0;
        }
        if (m_ColorTextureId != 0)
        {
            glDeleteTextures(1, &m_ColorTextureId);
            m_ColorTextureId = 0;
        }
        if (m_FBO != 0)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        m_Width = 0;
        m_Height = 0;
    }

    void FramebufferObject::Resize(std::uint32_t width, std::uint32_t height)
    {
        if (width == m_Width && height == m_Height)
            return;
        if (width == 0 || height == 0)
            return;
        Shutdown();
        Initialize(width, height);
    }

    void FramebufferObject::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void FramebufferObject::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    FramebufferObject::~FramebufferObject()
    {
        Shutdown();
    }
}
