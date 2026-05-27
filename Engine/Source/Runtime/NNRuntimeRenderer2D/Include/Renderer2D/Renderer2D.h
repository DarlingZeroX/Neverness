#pragma once

/**
 * @file Renderer2D.h
 * @brief 最小 2D 渲染器：接收 SpriteDrawCommand[]，通过 RHI（OpenGL）批量绘制 Quad。
 *
 * 职责：
 *   - 编译/管理 Builtin Sprite Shader
 *   - 管理 Unit Quad 几何体（VAO/VBO/EBO）
 *   - 管理 1x1 白色默认纹理
 *   - 批量提交 SpriteDrawCommand → glDrawElements
 *
 * 设计约束：
 *   - 不暴露 GLuint 到外部
 *   - 内部直接使用 OpenGL（MVP 阶段可接受）
 *   - 未来迁移到 Diligent 时仅需修改此类内部实现
 */

#include "SpriteDrawCommand.h"
#include "CameraData.h"
#include "Renderer2DExport.h"
#include <vector>
#include <cstdint>

namespace NN::Runtime::Renderer2D
{
    /// 2D 渲染器
    class NN_RUNTIME_RENDERER2D_API Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();

        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;

        /// 初始化：编译 Builtin Shader、创建 Quad VAO/VBO/EBO、创建白色默认纹理
        bool Initialize();
        void Shutdown();

        /// 开始一帧渲染
        void BeginScene(const CameraData& camera, std::uint32_t width, std::uint32_t height);

        /// 提交绘制命令数组
        void Submit(const std::vector<SpriteDrawCommand>& commands);

        /// 结束一帧渲染（Flush 剩余 Batch）
        void EndScene();

        /// 获取本帧 DrawCall 数量
        std::uint32_t GetDrawCallCount() const;
        /// 获取本帧绘制的 Quad 数量
        std::uint32_t GetQuadCount() const;

    private:
        struct Impl;
        Impl* m_Impl;
    };
}
