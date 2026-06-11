#pragma once

/**
 * @file SceneRenderer.h
 * @brief 场景渲染器：顶层入口，串联 ECS → SpriteRenderSystem → Renderer2D → Framebuffer。
 *
 * 对外只暴露一个接口：Render(scene, width, height) → TextureHandle (uint64_t)
 * C# EditorViewport 通过 NNNativeEngineAPI 调用此接口。
 */

#include "Renderer2D.h"
#include "SpriteRenderSystem.h"
#include "FramebufferObject.h"
#include "CameraData.h"
#include "Renderer2DExport.h"
#include <cstdint>
#include <vector>

namespace NN::Runtime::Scene
{
    class NNRuntimeScene;
}

namespace NN::Runtime::Render
{
    class INNRenderDevice;
}

namespace NN::Runtime::Renderer2D
{
    /// 场景渲染器
    class NN_RUNTIME_RENDERER2D_API SceneRenderer
    {
    public:
        SceneRenderer();
        ~SceneRenderer();

        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;

        /// 初始化
        /// @param device Diligent 渲染设备（由 NNRenderBootstrap 创建）
        bool Initialize(Render::INNRenderDevice* device);
        void Shutdown();

        /// 渲染场景到 Framebuffer，返回 Color Texture 的句柄
        /// 返回 reinterpret_cast<uint64_t>(ITextureView*)
        std::uint64_t Render(Scene::NNRuntimeScene& scene,
                             std::uint32_t width, std::uint32_t height);

        /// 获取上次渲染的纹理句柄
        std::uint64_t GetOutputTextureHandle() const;

        /// 视口尺寸变更
        void OnViewportResize(std::uint32_t width, std::uint32_t height);

        /// 获取本帧 DrawCall 数量
        std::uint32_t GetDrawCallCount() const;
        /// 获取本帧绘制的 Quad 数量
        std::uint32_t GetQuadCount() const;

        /// 获取 FramebufferObject（供 RmlUI 叠加渲染使用）
        FramebufferObject* GetFramebufferObject();

    private:
        /// 从场景中找到主相机，计算 CameraData
        CameraData FindMainCamera(Scene::NNRuntimeScene& scene);

        Renderer2D          m_Renderer;
        SpriteRenderSystem  m_SpriteSystem;
        FramebufferObject   m_Framebuffer;
        std::vector<SpriteDrawCommand> m_Commands;
        bool                m_Initialized = false;
    };
}
