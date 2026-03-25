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
#include "Interface/GalgameInterface.h"
#include "VGCore/Interface/AppInterface.h"
#include "../VGGalgameConfig.h"
#include "VGEngine/Include/Scene/Scene.h"
#include "VGEngine/Include/Render/RenderCore.h"
//#include "../Graphics/OpenGL/RenderTarget.h"
#include <VGRHI/Include/OpenGL/RenderTarget.h>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API RenderPipeline
	{
	public:
		RenderPipeline();
		RenderPipeline(const RenderPipeline&) = default;
		RenderPipeline& operator=(const RenderPipeline&) = default;
		RenderPipeline(RenderPipeline&&) noexcept = default;
		RenderPipeline& operator=(RenderPipeline&&) noexcept = default;
		~RenderPipeline() = default;

		void Initialize(IGameEngineContext* context);

		void Render(ILayeredSceneManager* scene, IOrthoCamera* camera, OpenGL::RenderTarget2D* rt);
		void OnScreenSizeChanged(int width, int height);
		void CaptureBackgroundLayer();
		void CaptureSceneLayer();
		void SetScene(Scene* scene);
	private:
		void CreatePrevFrameTexture(int width, int height);
		void CreateRenderTargets(int width, int height);

		void RenderSprite(IGameActor* actor, IOrthoCamera* camera);
		void RenderFullScreen(FullScreenRendererComponent* renderer);

		void RenderBackgroundLayer(ILayeredSceneManager* galScene, IOrthoCamera* camera);
		void RenderSceneLayer(ILayeredSceneManager* galScene, IOrthoCamera* camera, OpenGL::RenderTarget2D* rt);
		void RenderMixCharacterSprite();
	private:
		//IGameAppContext* m_AppContext;
		IGameEngineContext* m_GameEngineContext = nullptr;

		Ref<OpenGL::RenderTarget2D> m_BackgroundRT;
		Ref<OpenGL::RenderTarget2D> m_SceneRT;
		Ref<OpenGL::RenderTarget2D> m_SceneCharacterSpriteCurrentRT;
		Ref<OpenGL::RenderTarget2D> m_SceneCharacterSpritePrevRT;

		Ref<FullScreenRendererComponent> m_FullScreenRenderer;

		Ref<VGFX::ITexture> m_PrevBackgroundTexture;
		Ref<VGFX::ITexture> m_PrevSceneTexture;

		Scene* m_pScene;
	};
}
