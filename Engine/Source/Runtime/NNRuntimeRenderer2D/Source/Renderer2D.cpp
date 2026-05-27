/**
 * @file Renderer2D.cpp
 * @brief 最小 2D 渲染器实现：Builtin Shader 编译、Quad 几何体、Batch Draw。
 *
 * 内部直接使用 OpenGL（MVP 阶段），对外不暴露 GLuint。
 */

#include "Renderer2D/Renderer2D.h"
#include "Renderer2D/BuiltinShaders.h"
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>
#include <NNRuntimeRHI/Include/OpenGL/IncludeGladGL3.h>
#include <cstring>

namespace NN::Runtime::Renderer2D
{
    // ── Unit Quad 顶点数据 ──
    // 每个顶点：Position(x,y,z) + UV(u,v) = 5 floats
    static constexpr float QuadVertices[] = {
        // x      y      z     u     v
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 右下
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // 右上
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  // 左上
    };

    static constexpr unsigned int QuadIndices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    struct Renderer2D::Impl
    {
        std::uint32_t SpriteShaderProgram = 0;
        std::uint32_t QuadVAO = 0;
        std::uint32_t QuadVBO = 0;
        std::uint32_t QuadEBO = 0;
        std::uint32_t WhiteTexture = 0;

        std::uint32_t DrawCallCount = 0;
        std::uint32_t QuadCount = 0;

        // Uniform 位置缓存
        int Loc_ViewProjection = -1;
        int Loc_Transform = -1;
        int Loc_UvRect = -1;
        int Loc_FlipX = -1;
        int Loc_FlipY = -1;
        int Loc_Texture = -1;
        int Loc_Color = -1;
    };

    // ── 辅助函数：编译单个 Shader ──
    static std::uint32_t CompileShader(unsigned int type, const char* source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            // TODO: 输出到引擎日志系统
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    // ── 辅助函数：编译 Sprite Shader Program ──
    static std::uint32_t CompileBuiltinSpriteShader()
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, BuiltinShaders::SpriteVS);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, BuiltinShaders::SpriteFS);

        if (vs == 0 || fs == 0)
        {
            if (vs) glDeleteShader(vs);
            if (fs) glDeleteShader(fs);
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

    // ── 辅助函数：创建 1x1 白色纹理 ──
    static std::uint32_t CreateWhiteTexture1x1()
    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        const unsigned char white[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, white);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return tex;
    }

    // ── 辅助函数：创建 Quad 几何体 ──
    static void CreateQuadGeometry(std::uint32_t& vao, std::uint32_t& vbo, std::uint32_t& ebo)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices), QuadIndices, GL_STATIC_DRAW);

        // Position (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), reinterpret_cast<void*>(0));

        // TexCoord (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

        glBindVertexArray(0);
    }

    // ── 辅助函数：设置混合模式 ──
    static void ApplyBlendMode(BlendMode mode)
    {
        switch (mode)
        {
        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::Multiply:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
        case BlendMode::Opaque:
            glDisable(GL_BLEND);
            break;
        case BlendMode::Premultiplied:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
    }

    // ── Renderer2D 公开接口 ──

    Renderer2D::Renderer2D()
        : m_Impl(nullptr)
    {
    }

    Renderer2D::~Renderer2D()
    {
        Shutdown();
    }

    bool Renderer2D::Initialize()
    {
        m_Impl = new Impl();

        // 编译 Builtin Sprite Shader
        m_Impl->SpriteShaderProgram = CompileBuiltinSpriteShader();
        if (m_Impl->SpriteShaderProgram == 0)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }

        // 缓存 Uniform 位置
        glUseProgram(m_Impl->SpriteShaderProgram);
        m_Impl->Loc_ViewProjection = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_ViewProjection");
        m_Impl->Loc_Transform      = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_Transform");
        m_Impl->Loc_UvRect         = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_UvRect");
        m_Impl->Loc_FlipX          = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_FlipX");
        m_Impl->Loc_FlipY          = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_FlipY");
        m_Impl->Loc_Texture        = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_Texture");
        m_Impl->Loc_Color          = glGetUniformLocation(m_Impl->SpriteShaderProgram, "u_Color");
        glUseProgram(0);

        // 创建 Quad 几何体
        CreateQuadGeometry(m_Impl->QuadVAO, m_Impl->QuadVBO, m_Impl->QuadEBO);

        // 创建 1x1 白色纹理
        m_Impl->WhiteTexture = CreateWhiteTexture1x1();

        return true;
    }

    void Renderer2D::Shutdown()
    {
        if (m_Impl)
        {
            if (m_Impl->WhiteTexture)
            {
                glDeleteTextures(1, &m_Impl->WhiteTexture);
            }
            if (m_Impl->QuadVAO)
            {
                glDeleteVertexArrays(1, &m_Impl->QuadVAO);
            }
            if (m_Impl->QuadVBO)
            {
                glDeleteBuffers(1, &m_Impl->QuadVBO);
            }
            if (m_Impl->QuadEBO)
            {
                glDeleteBuffers(1, &m_Impl->QuadEBO);
            }
            if (m_Impl->SpriteShaderProgram)
            {
                glDeleteProgram(m_Impl->SpriteShaderProgram);
            }
            delete m_Impl;
            m_Impl = nullptr;
        }
    }

    void Renderer2D::BeginScene(const CameraData& camera, std::uint32_t width, std::uint32_t height)
    {
        if (!m_Impl) return;

        m_Impl->DrawCallCount = 0;
        m_Impl->QuadCount = 0;

        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        // 使用 Sprite Shader
        glUseProgram(m_Impl->SpriteShaderProgram);

        // 设置 ViewProjection 矩阵
        glUniformMatrix4fv(m_Impl->Loc_ViewProjection, 1, GL_FALSE,
                           camera.ViewProjectionMatrix);

        // 绑定 Quad VAO
        glBindVertexArray(m_Impl->QuadVAO);

        // 纹理单元 0
        glUniform1i(m_Impl->Loc_Texture, 0);

		H_LOG_INFO("--------------Sprite Loc_ViewProjection");
		H_LOG_INFO("ViewProjection1: %f, %f, %f, %f", camera.ViewProjectionMatrix[0], camera.ViewProjectionMatrix[1], camera.ViewProjectionMatrix[2], camera.ViewProjectionMatrix[3]);
		H_LOG_INFO("ViewProjection2: %f, %f, %f, %f", camera.ViewProjectionMatrix[4], camera.ViewProjectionMatrix[5], camera.ViewProjectionMatrix[6], camera.ViewProjectionMatrix[7]);
		H_LOG_INFO("ViewProjection3: %f, %f, %f, %f", camera.ViewProjectionMatrix[8], camera.ViewProjectionMatrix[9], camera.ViewProjectionMatrix[10], camera.ViewProjectionMatrix[11]);
		H_LOG_INFO("ViewProjection4: %f, %f, %f, %f", camera.ViewProjectionMatrix[12], camera.ViewProjectionMatrix[13], camera.ViewProjectionMatrix[14], camera.ViewProjectionMatrix[15]);
		H_LOG_INFO("--------------x");
    }

    void Renderer2D::Submit(const std::vector<SpriteDrawCommand>& commands)
    {
        if (!m_Impl) return;

        BlendMode currentBlend = BlendMode::Alpha;
        ApplyBlendMode(currentBlend);

        for (const auto& cmd : commands)
        {
            // 切换混合模式
            if (cmd.Blend != currentBlend)
            {
                currentBlend = cmd.Blend;
                ApplyBlendMode(currentBlend);
            }

            // 设置 Transform 矩阵
            glUniformMatrix4fv(m_Impl->Loc_Transform, 1, GL_FALSE, cmd.Transform);

			H_LOG_INFO("--------------Sprite Transform");
			H_LOG_INFO("Transform1: %f, %f, %f, %f", cmd.Transform[0], cmd.Transform[1], cmd.Transform[2], cmd.Transform[3]);
			H_LOG_INFO("Transform2: %f, %f, %f, %f", cmd.Transform[4], cmd.Transform[5], cmd.Transform[6], cmd.Transform[7]);
			H_LOG_INFO("Transform3: %f, %f, %f, %f", cmd.Transform[8], cmd.Transform[9], cmd.Transform[10], cmd.Transform[11]);
			H_LOG_INFO("Transform4: %f, %f, %f, %f", cmd.Transform[12], cmd.Transform[13], cmd.Transform[14], cmd.Transform[15]);
			H_LOG_INFO("--------------x");

            // 设置 Color
            glUniform4fv(m_Impl->Loc_Color, 1, cmd.Color);

            // 设置 UV Rect
            glUniform4fv(m_Impl->Loc_UvRect, 1, cmd.UvRect);

            // 设置 Flip
            glUniform1i(m_Impl->Loc_FlipX, cmd.FlipX ? 1 : 0);
            glUniform1i(m_Impl->Loc_FlipY, cmd.FlipY ? 1 : 0);

			H_LOG_INFO("--------------Sprite TextureHandle");
            // 绑定纹理（TextureHandle 已在 SpriteRenderSystem 中解析为 GL texture ID）
            glActiveTexture(GL_TEXTURE0);
            if (cmd.TextureHandle == 0)
            {
                glBindTexture(GL_TEXTURE_2D, m_Impl->WhiteTexture);
				H_LOG_INFO("WhiteTexture");
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(cmd.TextureHandle));
				H_LOG_INFO("TextureHandle(GL): %u", static_cast<GLuint>(cmd.TextureHandle));
            }
			H_LOG_INFO("--------------");

            // 绘制 Quad
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

            m_Impl->DrawCallCount++;
            m_Impl->QuadCount++;
        }
    }

    void Renderer2D::EndScene()
    {
        if (!m_Impl) return;

        glBindVertexArray(0);
        glUseProgram(0);
    }

    std::uint32_t Renderer2D::GetDrawCallCount() const
    {
        return m_Impl ? m_Impl->DrawCallCount : 0;
    }

    std::uint32_t Renderer2D::GetQuadCount() const
    {
        return m_Impl ? m_Impl->QuadCount : 0;
    }
}
