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
#include "NNRuntimeCore/Include/Core/Viewport.h"
#include "NNRuntimeCore/Interface/GameEngineInterface.h"
//#include "../Graphics/OpenGL/RenderTarget.h"
#include <NNRuntimeRHI/Include/OpenGL/RenderTarget.h>

namespace NN::Runtime
{
    class CoreRenderPipeline: public IRenderPipeline
    {
    public:
        CoreRenderPipeline() = default;
        ~CoreRenderPipeline() override = default;
        CoreRenderPipeline(const CoreRenderPipeline&) = delete;
        CoreRenderPipeline& operator=(const CoreRenderPipeline&) = delete;
        CoreRenderPipeline(CoreRenderPipeline&&) noexcept = default;
        CoreRenderPipeline& operator=(CoreRenderPipeline&&) noexcept = default;

        void Initialize(IGameEngineContext* context);

        void SetScene(IScene* scene);
        void SetViewport(Viewport* viewport);
        void OnUpdate() override;
        void OnRender() override;

		void RenderScene(IScene* scene,ICamera* camera ,uint pipelineIndex);

        void CreateRenderTargets(float2 size);

        VGFX::ITexture* GetRenderResult();
    private:
        IGameEngineContext* m_GameEngineContext = nullptr;
        IScene* m_Scene = nullptr;
        Viewport* m_Viewport = nullptr;

        Ref<OpenGL::RenderTarget2D> m_RenderBufferFinal;
    };
}
