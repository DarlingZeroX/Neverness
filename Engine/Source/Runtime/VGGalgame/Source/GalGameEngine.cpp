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

#include "GalGameEngine.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"
#include "VGGalgameCore/Include/Components.h"
#include "SpriteAnimationScriptManager.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGEngine/Include/Render/TransitionManager.h"
#include "VGEngine/Include/Scene/Scene.h"
#include "VGEngine/Include/Engine/Manager.h"

namespace VisionGal::GalGame
{
	GalGameEngine::GalGameEngine()
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

	void GalGameEngine::ReloadStoryScript()
	{
		m_StoryScriptSystem->ReloadStoryScript();
	}

	bool GalGameEngine::LoadStoryScript(const String& path)
	{
		m_StoryScriptSystem->LoadStoryScript(path);
		return true;
	}

	void GalGameEngine::LoadStoryScriptOnUpdate(const String& path)
	{
		m_StoryScriptSystem->LoadStoryScriptOnUpdate(path);
	}

	bool GalGameEngine::LoadArchive(const SaveArchive& archive)
	{
		return m_StoryScriptSystem->LoadArchive(archive);
	}

	void GalGameEngine::Reset()
	{
	}

	void GalGameEngine::Wait(float duration)
	{
		m_StoryScriptSystem->Wait(duration);
	}

	void GalGameEngine::CaptureSceneImage()
	{
		m_GalGameContext->runtimeState.screenshotPixels = MakeRef<VGFX::TexturePixels>();
		m_EngineContext->GetViewport()->GetViewportTexture()->ReadPixels(*m_GalGameContext->runtimeState.screenshotPixels);
	}

	ArchiveDataContainer* GalGameEngine::GetArchiveDataContainer() const
	{
		return m_GalGameContext->archiveData.get();
	}

	void GalGameEngine::OnMainSceneChanged(const EngineEvent& evt)
	{
		//Reset();
		m_LayeredSceneManager->ClearAll();

		//m_DialogueSystem->ClearDialogList();
		// 必须在更换场景时清除回调，因为回调是属于上一个场景，遗留调用会出错
		//m_DialogueSystem->ClearAllTypingCallbacks();		
		//m_DialogueSystem->FastForward(false);
		//m_DialogueSystem->AutoDialogue(false);

		// 先设置场景
		m_Scene = dynamic_cast<Scene*>(evt.Scene);
		m_RenderPipeline->SetScene(m_Scene);
		// 对话系统
		m_DialogueSystem->Clear();

		// 加载脚本
		if (GetSceneManager()->IsPlayMode())
		{
			// 这里需要在更新时加载脚本，因为场景更换可能会有上一个场景残留在资源系统，避免切换场景时脚本加载过快导致的问题
			m_IsEngineEnable = m_StoryScriptSystem->LoadSceneStoryScriptOnUpdate(evt.Scene);
		}
	}

	void GalGameEngine::CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext)
	{
		m_GalGameContext = MakeRef<GalGameContext>();

		// 初始化对话系统
		m_DialogueSystem = MakeRef<DialogueSystem>();
		m_DialogueSystem->InitialiseDataModel(uiContext);
		m_DialogueSystem->Initialize(m_GalGameContext);

		// 初始化分层场景管理器
		m_LayeredSceneManager = MakeRef<LayeredSceneSystem>();
		m_LayeredSceneManager->Initialize(m_GalGameContext);

		// 初始化渲染管线
		m_RenderPipeline = MakeRef<RenderPipeline>();
		m_RenderPipeline->Initialize(context);

		// 初始化存档系统
		m_ArchiveSystem = MakeRef<ArchiveSystem>();
		m_ArchiveSystem->Initialise(m_GalGameContext);

		// 初始剧情脚本系统
		m_StoryScriptSystem = MakeRef<StoryScriptSystem>();
		m_StoryScriptSystem->SetEngine(this);
		m_StoryScriptSystem->Initialise(m_GalGameContext, context);

		// 初始资源系统
		m_ResourceSystem = MakeRef<ResourceSystem>();
		m_ResourceSystem->Initialize(m_GalGameContext, m_LayeredSceneManager);

		// 初始界面系统
		m_GalGameUISystem = MakeRef<GalGameUISystem>();
		m_GalGameUISystem->Initialize(m_GalGameContext, context);
	}

	void GalGameEngine::Initialize(IGameEngineContext* context)
	{
		m_EngineContext = context;
		GameEngineCore::SetCurrentEngine(this);
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

	bool GalGameEngine::PreLoadResource(const String& path)
	{
		return m_ResourceSystem->PreLoadResource(path);
	}

	IGalSprite* GalGameEngine::ShowSprite(const std::string& layer, const std::string& path)
	{
		return m_ResourceSystem->ShowSprite(layer, path);
	}

	IGalSprite* GalGameEngine::ShowColor(const std::string& layer, const float4& color)
	{
		return m_ResourceSystem->ShowColor(layer, color);
	}

	IGalAudio* GalGameEngine::PlayAudio(const std::string& layer, const std::string& path)
	{
		return m_ResourceSystem->PlayAudio(layer, path);
	}

	IGalVideo* GalGameEngine::PlayVideo(const std::string& layer, const std::string& path)
	{
		return m_ResourceSystem->PlayVideo(layer, path);
	}

	IGalCharacter* GalGameEngine::CreateCharacter(const String& name)
	{
		GalCharacter* character = new GalCharacter(this, name);

		m_LayeredSceneManager->AddCharacter(character);

		return character;
	}

	void GalGameEngine::HideAllCharacterSprite()
	{
		m_LayeredSceneManager->TraverseCharacter([this](IGalCharacter* character)
			{
				GalCharacter* galChar = dynamic_cast<GalCharacter*>(character);
				galChar->HideFigure();
			});
	}

	bool GalGameEngine::RemoveSprite(IGalSprite* sprite)
	{
		return m_LayeredSceneManager->GetSpriteManager()->RemoveSprite(sprite);
	}

	bool GalGameEngine::RemoveAudio(IGalAudio* audio)
	{
		return m_LayeredSceneManager->GetAudioManager()->RemoveAudio(audio);
	}

	IArchiveSystem* GalGameEngine::GetArchiveSystem()
	{
		return m_ArchiveSystem.get();
	}

	IDialogueSystem* GalGameEngine::GetDialogueSystem()
	{
		return m_DialogueSystem.get();
	}

	ILayeredSceneManager* GalGameEngine::GetLayeredSceneManager()
	{
		return m_LayeredSceneManager.get();
	}

	IStoryScriptSystem* GalGameEngine::GetStoryScriptSystem()
	{
		return m_StoryScriptSystem.get();
	}

	IGalGameUISystem* GalGameEngine::GetGalGameUISystem()
	{
		return m_GalGameUISystem.get();
	}

	void GalGameEngine::OnRender()
	{}

	void GalGameEngine::OneRenderSceneCallback(OpenGL::RenderTarget2D* rt)
	{
		if (m_IsEngineEnable == false)
			return;

		auto& cameras = m_EngineContext->GetViewport()->GetCameras();
		ICamera* icamera;
		for (auto cam : cameras)
			icamera = cam;

		//auto* cameraCom = m_CameraActor->GetComponent<CameraComponent>();
		auto* camera = dynamic_cast<Letterbox2DCamera*>(icamera);

		m_RenderPipeline->Render(m_LayeredSceneManager.get(), camera, rt);
	}

	void GalGameEngine::OnUpdate(float deltaTime)
	{
		if (m_IsEngineEnable == false)
			return;

		m_LayeredSceneManager->OnUpdate();
		// 更新对话系统
		GetDialogueSystem()->Update();
		// 更新脚本系统
		m_StoryScriptSystem->Update();
	}
}
