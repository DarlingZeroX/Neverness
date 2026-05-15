/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzeroox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "GalGameEngine.h"
#include "VGGalgameCore/Include/GalGameEngineAccess.h"
#include "VGGalgameCore/Include/Components.h"
#include "SpriteAnimationScriptManager.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGEngine/Include/Render/TransitionManager.h"
#include "VGEngine/Include/Scene/Scene.h"
#include "VGEngine/Include/Engine/Manager.h"

namespace VisionGal::GalGame
{
	GalGameEngine::GalGameEngine()
		: m_SubsystemBus(this)
		, m_RuntimeHost(this)
	{
		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::MainSceneChanged:
					OnMainSceneChanged(evt);
					break;
				}
			});
	}

	GalGameEngine::~GalGameEngine()
	{
		/// 中文：在成员析构之前释放脚本/会话对 **this** 的隐含依赖；顺序与 **Shutdown** 文档一致。
		m_RuntimeCoordinator.Shutdown();
		GalGameEngineAccess::SetCurrent(nullptr);
	}

	void GalGameEngine::Reset()
	{
		m_RuntimeCoordinator.ResetRuntime();
	}

	void GalGameEngine::WaitForStoryScript(float duration)
	{
		if (m_StoryScriptSystem)
			m_StoryScriptSystem->Wait(duration);
	}

	ISubsystemBus* GalGameEngine::GetSubsystemBus()
	{
		return &m_SubsystemBus;
	}

	IGalGameContext* GalGameEngine::GetContext()
	{
		return m_GalGameContext.get();
	}

	IGalGameRuntime* GalGameEngine::GetRuntime() noexcept
	{
		return &m_RuntimeHost;
	}

	void GalGameEngine::OnMainSceneChanged(const EngineEvent& evt)
	{
		m_RuntimeCoordinator.HandleMainSceneChanged(evt);
	}

	void GalGameEngine::CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext)
	{
		m_RuntimeCoordinator.BeginSubsystemConstruction();

		/// 中文：Phase 8A-2 固定装配顺序 — **Context**（内含 **engineEventBus** / **uiEventBus**）→ **Systems**（场景/对白/渲染/存档/资源/UI）→ **ScriptRuntime**（**StoryScriptSystem**）→ **Execution**（**GalRuntimeSessionHost** 在 **Initialize** 末尾创建）。
		m_GalGameContext = GalGameContext::Create(&m_SubsystemBus);

		// 先完成 LayeredScene 对 engineEventBus 的订阅，再跑 Rml 数据模型绑定，避免第三方堆操作与上下文对象相邻分配时的调试期损坏（见编辑器 AV）。
		m_LayeredSceneManager = MakeRef<LayeredSceneSystem>();
		m_LayeredSceneManager->Initialize(m_GalGameContext);

		m_DialogueSystem = MakeRef<DialogueSystem>();
		m_DialogueSystem->InitialiseDataModel(uiContext);
		m_DialogueSystem->Initialize(m_GalGameContext);

		m_RenderPipeline = MakeRef<RenderPipeline>();
		m_RenderPipeline->Initialize(context);

		m_ArchiveSystem = MakeRef<ArchiveSystem>();
		m_ArchiveSystem->Initialise(m_GalGameContext);

		m_ResourceSystem = MakeRef<ResourceSystem>();
		m_ResourceSystem->Initialize(m_GalGameContext, m_LayeredSceneManager, this);

		m_GalGameUISystem = MakeRef<GalGameUISystem>();
		m_GalGameUISystem->Initialize(m_GalGameContext, context);

		m_StoryScriptSystem = MakeRef<StoryScriptSystem>();
		m_StoryScriptSystem->Initialise(m_GalGameContext, context, &m_SubsystemBus);

		m_RuntimeCoordinator.EndSubsystemConstruction();
	}

	void GalGameEngine::Initialize(IGameEngineContext* context)
	{
		m_EngineContext = context;
		m_RuntimeCoordinator.Attach(this);
		GalGameEngineAccess::SetCurrent(this);
		auto* rmlContext = static_cast<Rml::Context*>(context->GetUISystem()->GetContext());
		CreateSubsystem(context, rmlContext);

		context->GetViewport()->OnViewportEvent.Subscribe([this](const ViewportEvent& evt)
			{
				m_RenderPipeline->OnScreenSizeChanged(evt.NewViewportSize.x, evt.NewViewportSize.y);
			});

		context->AddBeforeRenderCallback("GalGameEngineRenderCallback", [this](OpenGL::RenderTarget2D* rt)
			{
				this->OneRenderSceneCallback(rt);
			});

		m_RuntimeSession = std::make_unique<GalRuntimeSessionHost>(this);
		m_RuntimeCoordinator.MarkHostRunning();
		m_RuntimeSession->Start();
	}

	bool GalGameEngine::TransitionCommand(const String& layer, const String& cmd)
	{
		if (m_DialogueSystem->IsFastForward())
			return true;

		return TransitionManager::GetInstance()->StartTransitionWithCommand(layer, cmd);
	}

	bool GalGameEngine::TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd)
	{
		if (m_DialogueSystem->IsFastForward())
			return true;

		String path = Core::GetAssetsPathVFS() + imagePath;
		return TransitionManager::GetInstance()->StartCustomImageTransitionWithCommand(layer, path, cmd);
	}

	void GalGameEngine::OnRender()
	{}

	void GalGameEngine::OneRenderSceneCallback(OpenGL::RenderTarget2D* rt)
	{
		if (m_IsEngineEnable == false)
			return;

		auto& cameras = m_EngineContext->GetViewport()->GetCameras();
		ICamera* icamera = nullptr;
		for (auto cam : cameras)
			icamera = cam;

		auto* camera = dynamic_cast<Letterbox2DCamera*>(icamera);

		m_RenderPipeline->Render(m_LayeredSceneManager.get(), camera, rt);
	}

	void GalGameEngine::OnUpdate(float deltaTime)
	{
		if (m_RuntimeSession)
			m_RuntimeSession->Tick(deltaTime);
	}

	IGalRuntimeSession* GalGameEngine::GetRuntimeSession() noexcept
	{
		return m_RuntimeSession.get();
	}
}
