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

#include "Game/RenderEngine.h"
#include "NNRuntimeRmlui/Interface/UISystemLegacy.h"
#include "Engine/Manager.h"
#include "Scene/Components.h"
#include "Render/TransitionManager.h"

namespace NN::Runtime
{
	CoreRenderEngine::CoreRenderEngine()
	{
	}

	void CoreRenderEngine::OnUpdate(float deltaTime)
	{
		m_RenderPipeline->OnUpdate();
		UISystemLegacy::Get()->OnUpdate();
	}

	void CoreRenderEngine::OnRender()
	{
		auto* scene = GetSceneManager()->GetCurrentRunningScene();

		if (scene == nullptr)
			return;

		// 清除颜色缓冲区（和深度缓冲区，若启用了深度测试）
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 先用渲染管线渲染场景
		m_RenderPipeline->SetScene(GetSceneManager()->GetCurrentRunningScene());
		m_RenderPipeline->OnRender();

		// 把场景作为UI背景,然后把UI渲染在上面,才能获得正确的UI渲染结果
		UISystemLegacy::Get()->BeginFrame();
		RenderUIBackground();
		UISystemLegacy::Get()->RenderContext();
		UISystemLegacy::Get()->EndFrame();
		//UISystem::Get()->Render();

		// 然后渲染到屏幕渲染目标
		RenderToScreenRT();

		// 渲染到最终渲染目标
		RenderFinalResultRT();

		if (m_EnableRenderDirectToScreen)
			RenderFinalResultToScreen();
	}

	void CoreRenderEngine::RenderUIBackground()
	{
		auto program = GetShaderManager()->GetBuiltinProgram("FullScreenQuad");
		auto* mesh = FullScreenQuad::GetFullScreenQuadMesh();

		VGFX::SetViewRect(0, 0, m_Viewport->Width(), m_Viewport->Height());

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "texture1", m_RenderPipeline->GetRenderResult());
		VGFX::RenderMesh(mesh);
	}

	void CoreRenderEngine::RenderFinalResultToScreen()
	{
		if (m_FinalResultRT == nullptr)
			return;

		auto program = GetShaderManager()->GetBuiltinProgram("FullScreenQuad");
		auto* mesh = FullScreenQuad::GetFullScreenQuadMesh();

		VGFX::SetViewRect(0, 0, m_Viewport->Width(), m_Viewport->Height());

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "texture1", m_FinalResultRT->GetTexture());
		VGFX::RenderMesh(mesh);
	}

	void CoreRenderEngine::RenderToScreenRT()
	{
		if (m_ScreenRT == nullptr)
			return;

		auto program = GetShaderManager()->GetBuiltinProgram("ViewportOutput");
		auto* mesh = FullScreenQuad::GetFullScreenQuadMesh();

		VGFX::SetViewRect(0, 0, m_Viewport->Width(), m_Viewport->Height());
		m_ScreenRT->GetFrameBuffer()->Bind();

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "renderTex", m_RenderPipeline->GetRenderResult());
		VGFX::SetTexture2DNative(1, "uiTex", UISystemLegacy::Get()->GetUIRenderResult());
		VGFX::RenderMesh(mesh);

		// 渲染屏幕前场景
		if (m_Viewport->GetCameras().empty() == false)
		{
			auto& cameras = m_Viewport->GetCameras();
			ICamera* camera;
			for (auto cam : cameras)
				camera = cam;
			auto* orthoCamera = dynamic_cast<IOrthoCamera*>(camera);
			if (orthoCamera)
			{
				auto* scene = GetSceneManager()->GetCurrentRunningScene();
				if (scene)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_RenderPipeline->RenderScene(scene, orthoCamera, static_cast<uint>(RenderPipelineIndex::ScreenPipeline));
					glDisable(GL_BLEND);
				}
			}
		}

		m_ScreenRT->GetFrameBuffer()->Unbind();

	//	m_Viewport->SetViewportTexture(m_ScreenRT->GetTexture());
	}

	void CoreRenderEngine::RenderFinalResultRT()
	{
		if (m_ScreenRT == nullptr)
			return;

		if (m_FinalResultRT == nullptr)
			return;

		auto program = GetShaderManager()->GetBuiltinProgram("FullScreenQuad");
		auto* mesh = FullScreenQuad::GetFullScreenQuadMesh();

		VGFX::SetViewRect(0, 0, m_Viewport->Width(), m_Viewport->Height());

		// 渲染到最终结果
		m_FinalResultRT->GetFrameBuffer()->Bind();
		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "texture1", m_ScreenRT->GetTexture());
		VGFX::RenderMesh(mesh);
		m_FinalResultRT->GetFrameBuffer()->Unbind();

		// 屏幕层转场
		if (m_PrevScreenTexture)
		{
			m_FinalResultRT->GetFrameBuffer()->Bind();
			TransitionManager::GetInstance()->LayerTransition(
				"Screen",
				m_PrevScreenTexture,
				m_ScreenRT->GetTextureRef()
			);
			m_FinalResultRT->GetFrameBuffer()->Unbind();
		}

		m_Viewport->SetViewportTexture(m_FinalResultRT->GetTexture());
	}

	void CoreRenderEngine::CaptureScreenLayer()
	{
		if (!m_PrevScreenTexture)
			return;

		m_FinalResultRT->CopyToTexture(m_PrevScreenTexture.get());
	}

	void CoreRenderEngine::Initialize(IGameEngineContext* context)
	{
		m_GameEngineContext = context;

		// 获取视口
		m_Viewport = context->GetViewport();

		// 创建渲染管线
		m_RenderPipeline = MakeRef<CoreRenderPipeline>();
		m_RenderPipeline->Initialize(context);
		m_RenderPipeline->SetViewport(m_Viewport);

		// 订阅视口大小变换事件
		m_Viewport->OnViewportEvent.Subscribe([this](const ViewportEvent& evt)
			{
				OnViewportSizeChanged(evt.NewViewportSize.x, evt.NewViewportSize.y);
			});

		// 初始化UI系统
		UISystemLegacy::Get()->Initialize(context->GetWindow(), m_Viewport);

		// 订阅屏幕转场事件
		TransitionManager::GetInstance()->OnTransitionEvent.Subscribe([this](const TransitionEvent& event)
			{
				if (event.Layer == "Screen")
				{
					this->CaptureScreenLayer();
				}
			});
	}

	void CoreRenderEngine::OnViewportSizeChanged(int width, int height)
	{
		m_ScreenRT = OpenGL::RenderTarget2D::Create(width, height);
		m_FinalResultRT = OpenGL::RenderTarget2D::Create(width, height);

		VGFX::TextureDesc desc;
		desc.Width = m_GameEngineContext->GetViewport()->Width();
		desc.Height = m_GameEngineContext->GetViewport()->Height();
		desc.Data = nullptr;
		desc.Format = GL_RGBA;
		desc.InternalFormat = GL_RGBA;
		desc.Type = GL_UNSIGNED_BYTE;

		m_PrevScreenTexture = VGFX::CreateTextureFromMemory(desc);

		//设置视口的最终渲染结果纹理
		m_Viewport->SetViewportTexture(m_FinalResultRT->GetTexture());
	}

	void CoreRenderEngine::SetRenderFinalResultToScreen(bool enable)
	{
		m_EnableRenderDirectToScreen = true;
	}
}
