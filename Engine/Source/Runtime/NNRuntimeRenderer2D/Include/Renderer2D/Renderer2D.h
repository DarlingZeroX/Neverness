#pragma once

/**
 * @file Renderer2D.h
 * @brief 最小 2D 渲染器：接收 SpriteDrawCommand[]，通过 Diligent 后端批量绘制 Quad。
 *
 * 职责：
 *   - 编译/管理 Builtin Sprite Shader（GLSL，由 Diligent 编译为 SPIRV）
 *   - 管理 Unit Quad 几何体（VB/IB）
 *   - 管理 1x1 白色默认纹理
 *   - 批量提交 SpriteDrawCommand → DrawIndexed
 *
 * 设计约束：
 *   - 通过 INNRenderDevice 创建 GPU 资源
 *   - 通过 Diligent IDeviceContext 录制渲染命令
 *   - 保留 GLSL 着色器，不转换 HLSL
 */

#include "SpriteDrawCommand.h"
#include "CameraData.h"
#include "Renderer2DExport.h"
#include <vector>
#include <cstdint>

// 前向声明：避免暴露 Experiments 头文件到外部
namespace NN::Runtime::Render { class INNRenderDevice; }

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

        /// 初始化：编译 Builtin Shader、创建 Quad VB/IB、创建白色默认纹理
        /// @param device Diligent 渲染设备（由 NNRenderBootstrap 创建）
        bool Initialize(Render::INNRenderDevice* device);
        void Shutdown();

        /// 设置渲染目标（创建 Diligent Framebuffer 用于 BeginRenderPass）
        /// @param rtv 颜色附件 RTV
        /// @param dsv 深度模板附件 DSV（可为 nullptr）
        void SetRenderTarget(void* rtv, void* dsv);

        /// 开始一帧渲染
        void BeginScene(const CameraData& camera, std::uint32_t width, std::uint32_t height);

        /// 提交绘制命令数组
        void Submit(const std::vector<SpriteDrawCommand>& commands);

        /// 结束一帧渲染
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
