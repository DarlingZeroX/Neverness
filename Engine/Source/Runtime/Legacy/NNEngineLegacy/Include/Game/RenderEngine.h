/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "RenderPipeline.h"
#include "../EngineConfig.h"
#include "NNRuntimeCore/Include/Core/Viewport.h"

namespace NN::Runtime
{
    class VG_ENGINE_API CoreRenderEngine : public IGameEngine
    {
    public:
        CoreRenderEngine();
        ~CoreRenderEngine() override = default;
        CoreRenderEngine(const CoreRenderEngine&) = delete;
        CoreRenderEngine& operator=(const CoreRenderEngine&) = delete;
        CoreRenderEngine(CoreRenderEngine&&) noexcept = default;
        CoreRenderEngine& operator=(CoreRenderEngine&&) noexcept = default;

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
		
        void Initialize(IGameEngineContext* context);
        void OnViewportSizeChanged(int width, int height);
		void SetRenderFinalResultToScreen(bool enable);
		 
        /// <summary>
        /// 把场景渲染结果作为UI渲染背景渲染
        /// </summary>
        void RenderUIBackground();
        void RenderToScreenRT();
        void RenderFinalResultRT();

        void CaptureScreenLayer();
		void RenderFinalResultToScreen();
    private:
        IGameEngineContext* m_GameEngineContext = nullptr;
        Viewport* m_Viewport;
        Ref<CoreRenderPipeline> m_RenderPipeline;
        Ref<OpenGL::RenderTarget2D> m_ScreenRT;
        Ref<OpenGL::RenderTarget2D> m_FinalResultRT;

        Ref<VGFX::ITexture> m_PrevScreenTexture;

		bool m_EnableRenderDirectToScreen = false; // 是否直接渲染到屏幕
    };
}
