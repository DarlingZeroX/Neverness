/**
 * @file SceneRenderer.cpp
 * @brief 场景渲染器实现：串联 ECS → SpriteRenderSystem → Renderer2D → Framebuffer。
 */

#include "Renderer2D/SceneRenderer.h"
#include <Scene/NNRuntimeScene.h>
#include <Scene/NNWorld.h>
#include <Components/NNTransformComponent.h>
#include <Components/NNCameraComponent.h>
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>
#include <NNRuntimeRHI/Include/OpenGL/IncludeGladGL3.h>

// GLM 矩阵运算：inverse + 矩阵乘法
#include <NNCore/Include/Math/GLM/gtc/matrix_inverse.hpp>

#include <cstring>

namespace NN::Runtime::Renderer2D
{
    SceneRenderer::SceneRenderer() = default;
    SceneRenderer::~SceneRenderer()
    {
        Shutdown();
    }

    bool SceneRenderer::Initialize()
    {
        if (m_Initialized)
            return true;

        if (!m_Renderer.Initialize())
            return false;

        // 初始 Framebuffer 尺寸（会在 Render 时按需 Resize）
        if (!m_Framebuffer.Initialize(1280, 720))
        {
            m_Renderer.Shutdown();
            return false;
        }

        m_Initialized = true;
        return true;
    }

    void SceneRenderer::Shutdown()
    {
        if (!m_Initialized)
            return;

        m_Framebuffer.Shutdown();
        m_Renderer.Shutdown();
        m_Initialized = false;
    }

    std::uint32_t SceneRenderer::Render(
        Scene::NNRuntimeScene& scene,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!m_Initialized)
        {
			H_LOG_WARN("SceneRenderer not initialized");
			return 0;
        }

        if (width == 0 || height == 0)
            return m_Framebuffer.GetColorTextureId();

        // 1. 确保 Framebuffer 尺寸正确
        if (m_Framebuffer.GetWidth() != width || m_Framebuffer.GetHeight() != height)
        {
            m_Framebuffer.Resize(width, height);
        }

        // 2. 获取相机数据
        CameraData camera = FindMainCamera(scene);

        // 3. 收集绘制命令
        m_SpriteSystem.Collect(scene, m_Commands);

        // 4. 绑定 FBO
        m_Framebuffer.Bind();

        // 5. 清屏
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		H_LOG_INFO("Command count: %d", (int)m_Commands.size());

        // 6. 渲染
        m_Renderer.BeginScene(camera, width, height);
        m_Renderer.Submit(m_Commands);
        m_Renderer.EndScene();

        // 7. 解绑 FBO
        m_Framebuffer.Unbind();

		//H_LOG_INFO("Rendered scene");
        return m_Framebuffer.GetColorTextureId();
    }

    std::uint32_t SceneRenderer::GetOutputTextureId() const
    {
        return m_Framebuffer.GetColorTextureId();
    }

    void SceneRenderer::OnViewportResize(std::uint32_t width, std::uint32_t height)
    {
        m_Framebuffer.Resize(width, height);
    }

    std::uint32_t SceneRenderer::GetDrawCallCount() const
    {
        return m_Renderer.GetDrawCallCount();
    }

    std::uint32_t SceneRenderer::GetQuadCount() const
    {
        return m_Renderer.GetQuadCount();
    }

    CameraData SceneRenderer::FindMainCamera(Scene::NNRuntimeScene& scene)
    {
        CameraData data{};

        // 默认正交相机（单位矩阵）
        Core::matrix defaultVP{1.0f};
        std::memcpy(data.ViewMatrix, &defaultVP, sizeof(float) * 16);
        std::memcpy(data.ProjectionMatrix, &defaultVP, sizeof(float) * 16);
        std::memcpy(data.ViewProjectionMatrix, &defaultVP, sizeof(float) * 16);

        // 查询带有 CameraComponent 的实体
        auto& registry = scene.GetRegistry();
        auto view = registry.view<
            Scene::NNTransformComponent,
            Scene::NNCameraComponent>();

        for (auto [entity, transform, cameraComp] : view.each())
        {
            // ProjectionMatrix（由 NNCameraSystem 每帧计算）
            std::memcpy(data.ProjectionMatrix, &cameraComp.ProjectionMatrix,
                        sizeof(float) * 16);

            // ViewMatrix = 逆 WorldMatrix（GLM 提供 inverse）
            //
            // 注意：相机默认在 z=0，但 ortho 默认 near=0.3。
            // 右手坐标系相机看向 -Z，z=0 的 Sprite 在相机位置（z_view=0），
            // 位于近平面 z_view=-0.3 之后，NDC z = -1.0006 超出 [-1,1] 被裁剪。
            // 修复：将相机 Z 移到 +(near+far)/2 ≈ +500.15，
            // 使 z=0 的 Sprite 的 z_view = 0 - 500.15 = -500.15，落在 near/far 中间。
            Core::matrix worldMat = transform.WorldMatrix;
            float nearZ = cameraComp.NearPlane;
            float farZ  = cameraComp.FarPlane;
            float cameraZ = (nearZ + farZ) * 0.5f;  // +(0.3+1000)/2 ≈ +500.15
            worldMat[3][2] = cameraZ;

            Core::matrix viewMat = glm::inverse(worldMat);
            std::memcpy(data.ViewMatrix, &viewMat, sizeof(float) * 16);

            // ViewProjection = Projection * View
            Core::matrix vpMat = cameraComp.ProjectionMatrix * viewMat;
            std::memcpy(data.ViewProjectionMatrix, &vpMat, sizeof(float) * 16);

            data.OrthoWidth  = cameraComp.OrthoWidth;
            data.OrthoHeight = cameraComp.OrthoHeight;
            data.Near = cameraComp.NearPlane;
            data.Far  = cameraComp.FarPlane;
            break; // MVP 阶段只取第一个相机
        }

        return data;
    }
}
