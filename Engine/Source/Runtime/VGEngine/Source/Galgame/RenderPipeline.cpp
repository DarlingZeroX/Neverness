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

#include "Galgame/RenderPipeline.h"
#include "Engine/Manager.h"
#include "Interface/Loader.h"
#include "Render/TransitionManager.h"
#include "Scene/Scene.h"

namespace VisionGal::GalGame
{
	RenderPipeline::RenderPipeline()
	{
	}

	void RenderPipeline::CreatePrevFrameTexture(int width, int height)
	{
		VGFX::TextureDesc desc;
		desc.Width = m_GameEngineContext->GetViewport()->Width();
		desc.Height = m_GameEngineContext->GetViewport()->Height();
		desc.Data = nullptr;
		desc.Format = GL_RGBA;
		desc.InternalFormat = GL_RGBA;
		desc.Type = GL_UNSIGNED_BYTE;

		m_PrevBackgroundTexture = VGFX::CreateTextureFromMemory(desc);
		m_PrevSceneTexture = VGFX::CreateTextureFromMemory(desc);
	}

	void RenderPipeline::CreateRenderTargets(int width, int height)
	{
		m_BackgroundRT = OpenGL::RenderTarget2D::Create(width, height);
		m_SceneRT = OpenGL::RenderTarget2D::Create(width, height);
		m_SceneCharacterSpritePrevRT = OpenGL::RenderTarget2D::Create(width, height);
		m_SceneCharacterSpriteCurrentRT = OpenGL::RenderTarget2D::Create(width, height);
		//m_RenderTarget2DFinal = OpenGL::RenderTarget2D::Create(width, height);
	}

	void RenderPipeline::RenderSprite(GameActor* actor, IOrthoCamera* camera)
	{
		SpriteRendererHandler handler;
		handler.Render(actor, camera, static_cast<uint>(RenderPipelineIndex::GalGamePipeline));

		//TransformComponent* transform = actor->GetComponent<TransformComponent>();
		//SpriteRendererComponent* spriteRenderer = actor->GetComponent<SpriteRendererComponent>();
		//
		//auto* tex = spriteRenderer->sprite->GetITexture();
		//auto* program = spriteRenderer->material->GetShaderProgram();
		//
		//VGFX::UseProgram(program);
		//VGFX::SetTexture(0, "texture1", tex);
		//VGFX::SetUniformFloat4("color", spriteRenderer->color);
		//VGFX::SetUniformInt2("flip", spriteRenderer->flip);
		//VGFX::SetUniformMatrix4("model", transform->model);
		//VGFX::SetUniformMatrix4("projection", camera->GetMatrix());
		//VGFX::RenderMesh(spriteRenderer->sprite->GetMesh());
	}

	void RenderPipeline::RenderFullScreen(FullScreenRendererComponent* renderer)
	{
		VGFX::ITexture* tex = renderer->image->GetTexture();
		auto* program = renderer->shader;

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "texture1", tex);
		VGFX::RenderMesh(renderer->image->GetMesh());
	}

	void RenderPipeline::Initialize(IGameEngineContext* context)
	{
		m_GameEngineContext = context;

		int width = context->GetViewport()->Width();
		int height = context->GetViewport()->Height();
		CreateRenderTargets(width, height);
		CreatePrevFrameTexture(width, height);

		m_FullScreenRenderer = CreateRef<FullScreenRendererComponent>();
		//m_FullScreenRenderer->image->GetITexture()->SetTexture(m_RenderTarget2DFinal->GetTextureRef());

		TransitionManager::GetInstance()->OnTransitionEvent.Subscribe([this](const TransitionEvent& event)
			{
				if (event.Layer == "Background")
				{
					this->CaptureBackgroundLayer();
				}
				else if (event.Layer == "Foreground")
				{
					this->CaptureSceneLayer();
				}
			});
	}

	void RenderPipeline::RenderBackgroundLayer(ILayeredSceneManager* galScene, IOrthoCamera* camera)
	{
		// 渲染背景层
		m_BackgroundRT->GetFrameBuffer()->Bind();
		galScene->GetSpriteManager()->TraverseSpriteLayer("Background", [this, camera](IGalGameSprite* sprite)
			{
				RenderSprite(sprite->GetResourceActor(), camera);
			});
		m_BackgroundRT->GetFrameBuffer()->Unbind();

		// 把背景层结果渲染到场景层
		m_SceneRT->GetFrameBuffer()->Bind();
		m_FullScreenRenderer->image->SetTexture(m_BackgroundRT->GetTextureRef());
		RenderFullScreen(m_FullScreenRenderer.get());
		m_SceneRT->GetFrameBuffer()->Unbind();

		// 背景层场景转换
		m_SceneRT->GetFrameBuffer()->Bind();
		TransitionManager::GetInstance()->LayerTransition(
			"Background",
			m_PrevBackgroundTexture,
			m_BackgroundRT->GetTextureRef()
		);
		m_SceneRT->GetFrameBuffer()->Unbind();
	}

	void RenderPipeline::RenderSceneLayer(ILayeredSceneManager* galScene, IOrthoCamera* camera,
		OpenGL::RenderTarget2D* rt)
	{
		// 渲染人物立绘层
		{
			// RGB通道使用预乘Alpha混合，Alpha通道使用加法混合，在最后混合人物立绘层时，RGB通道因为预乘过直接相加，Alpha通道也使用加法混合
			glBlendFuncSeparate(
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,  // RGB通道混合因子
				GL_ONE, GL_ONE_MINUS_SRC_ALPHA         // Alpha通道混合因子
			);
			// 渲染之前人物立绘层
			m_SceneCharacterSpritePrevRT->GetFrameBuffer()->Bind();
			// 清除颜色缓冲区（和深度缓冲区，若启用了深度测试）
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			galScene->GetSpriteManager()->TraverseSpriteLayer("SceneCharacterSpritePrev", [this, camera](IGalGameSprite* sprite)
				{
					RenderSprite(sprite->GetResourceActor(), camera);
				});
			m_SceneCharacterSpritePrevRT->GetFrameBuffer()->Unbind();

			// 渲染当前人物立绘层
			m_SceneCharacterSpriteCurrentRT->GetFrameBuffer()->Bind();
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			galScene->GetSpriteManager()->TraverseSpriteLayer("SceneCharacterSpriteCurrent", [this, camera](IGalGameSprite* sprite)
				{
					RenderSprite(sprite->GetResourceActor(), camera);
				});
			m_SceneCharacterSpriteCurrentRT->GetFrameBuffer()->Unbind();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// 混合人物立绘层
			RenderMixCharacterSprite();
		}

		// 渲染前景层
		m_SceneRT->GetFrameBuffer()->Bind();
		galScene->GetSpriteManager()->TraverseSpriteLayer("Foreground", [this, camera](IGalGameSprite* sprite)
			{
				RenderSprite(sprite->GetResourceActor(), camera);
			});
		m_SceneRT->GetFrameBuffer()->Unbind();

		// 渲染到屏幕帧缓冲
		rt->GetFrameBuffer()->Bind();
		m_FullScreenRenderer->image->SetTexture(m_SceneRT->GetTextureRef());
		RenderFullScreen(m_FullScreenRenderer.get());
		rt->GetFrameBuffer()->Unbind();

		//if (ImGui::Begin("TestForeground"))
		//{
		//	ImGui::Text("PrevForeground");
		//	ImGui::Image((void*)(intptr_t)m_PrevSceneTexture->GetShaderResourceView(), ImVec2(256, 144),
		//		ImVec2(0, 1), ImVec2(1, 0));
		//
		//	ImGui::Text("NextForeground");
		//	ImGui::Image((void*)(intptr_t)m_SceneRT->GetTexture()->GetRendererID(), ImVec2(256, 144),
		//		ImVec2(0, 1), ImVec2(1, 0));
		//}
		//ImGui::End();
		
		// 场景层场景转换
		rt->GetFrameBuffer()->Bind();
		TransitionManager::GetInstance()->LayerTransition(
			"Foreground",
			m_PrevSceneTexture,
			m_SceneRT->GetTextureRef()
		);
		rt->GetFrameBuffer()->Unbind();
	}

	void RenderPipeline::RenderMixCharacterSprite()
	{
		//VGFX::UseProgram(m_ShaderProgram);
		//
		//switch (m_FadeType)
		//{
		//case FadeType::Fade:
		//	VGFX::SetUniformInt("fadeType", 0); // Fade
		//	break;
		//case FadeType::FadeIn:
		//	VGFX::SetUniformInt("fadeType", 1); // Fade In
		//	break;
		//case FadeType::FadeOut:
		//	VGFX::SetUniformInt("fadeType", 2); // Fade Out
		//	break;
		//}
		//
		//VGFX::SetTexture(0, "prevScene", GetPrevFrame());
		//VGFX::SetTexture(1, "nextScene", GetNextFrame());
		m_SceneRT->GetFrameBuffer()->Bind();
		auto program = GetShaderManager()->GetBuiltinProgram("MixCharacterSprite");
		auto* mesh = FullScreenQuad::GetFullScreenQuadMesh();

		//VGFX::SetViewRect(0, 0, m_Viewport->Width(), m_Viewport->Height());

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "prevScene", m_SceneCharacterSpritePrevRT->GetTexture());
		VGFX::SetTexture(1, "nextScene", m_SceneCharacterSpriteCurrentRT->GetTexture());
		VGFX::RenderMesh(mesh);

		m_SceneRT->GetFrameBuffer()->Unbind();
	}

	void RenderPipeline::Render(ILayeredSceneManager* traverser, IOrthoCamera* camera, OpenGL::RenderTarget2D* rt)
	{
		if (m_pScene == nullptr)
		{
			H_LOG_ERROR("当前GalGame管线场景为空");
			return;
		}

		auto width = m_GameEngineContext->GetViewport()->Width();
		auto height = m_GameEngineContext->GetViewport()->Height();
		VGFX::SetViewRect(0, 0, width, height);

		// 清空自定义Framebuffer
		m_BackgroundRT->GetFrameBuffer()->Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_BackgroundRT->GetFrameBuffer()->Unbind();

		m_SceneRT->GetFrameBuffer()->Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_SceneRT->GetFrameBuffer()->Unbind();

		{
			// 渲染背景层
			RenderBackgroundLayer(traverser, camera);

			// Set blending function for premultiplied alpha.
			glEnable(GL_BLEND);
			//glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// 渲染场景层
			RenderSceneLayer(traverser, camera, rt);

			glDisable(GL_BLEND);
		}
	}

	void RenderPipeline::OnScreenSizeChanged(int width, int height)
	{
		CreateRenderTargets(width, height);
		CreatePrevFrameTexture(width, height);
	}

	void RenderPipeline::CaptureBackgroundLayer()
	{
		m_BackgroundRT->CopyToTexture(m_PrevBackgroundTexture.get());
	}

	void RenderPipeline::CaptureSceneLayer()
	{
		m_SceneRT->CopyToTexture(m_PrevSceneTexture.get());
	}

	void RenderPipeline::SetScene(Scene* scene)
	{
		m_pScene = scene;
	}
}
