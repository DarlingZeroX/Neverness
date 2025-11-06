#include "Galgame/GalGameEngine.h"
#include "Interface/Loader.h"
#include "Scene/Scene.h"
#include "Galgame/GameEngineCore.h"
#include "Core/EventBus.h"
#include "Engine/Manager.h"
#include "Galgame/Components.h"
#include "Galgame/GameLua.h"
#include "Galgame/StoryScriptLuaInterface.h"
#include "Galgame/SpriteAnimationScriptManager.h"
#include "Render/TransitionManager.h"
#include "Engine/EngineResource.h"

#include "Resource/Texture/TextureConverter.h"

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
		AddUpdateCallback([this]()
			{
				Reset();

				// 加载脚本
				{
					auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();

					view.each([this](GalGameEngineComponent& com) { // flecs::entity argument is optional

						if (com.script)
						{
							com.script = LuaStoryScript::LoadFromFile(com.script->GetResourcePath());
							m_StoryScript = com.script;
						}
						});

					if (m_StoryScript)
					{
						m_StoryScript->Run(this);
					}
				}
			});

	}

	bool GalGameEngine::LoadStoryScript(const String& path)
	{
		Reset();

		// 加载脚本
		auto storyScript = LuaStoryScript::LoadFromFile(path);

		if (storyScript == nullptr)
			return false;

		// 运行剧情脚本
		bool result = storyScript->Run(this);

		if (result == false)
			return false;

		m_StoryScript = storyScript;

		// 把脚本设置到引擎组件
		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();
		view.each([this](GalGameEngineComponent& com) { // flecs::entity argument is optional
			com.script = m_StoryScript;
			});

		return true;
	}

	void GalGameEngine::LoadStoryScriptOnUpdate(const String& path)
	{
		AddUpdateCallback([this, path]()
			{
				LoadStoryScript(path);
			});
	}

	bool GalGameEngine::LoadArchive(const SaveArchive& archive)
	{
		Reset();

		m_DialogueSystem->FastForward(true);

		if (!LoadStoryScript(archive.scriptPath))
		{
			m_DialogueSystem->FastForward(false);
			return false;
		}

		while (archive.line != m_DialogueSystem->GetCurrentDialogLine())
		{
			//DialogSystemLuaInterface::Continue();
			m_DialogueSystem->ContinueDialogue();
		}

		m_DialogueSystem->FastForward(false);

		return true;
	}

	void GalGameEngine::Reset()
	{
		m_Characters.clear();

		// 这里要放在m_Characters.clear();后面，因为m_Characters清除时候会删除Lua脚本的回调
		// 如果放在前面，Lua脚本的回调析构会出现异常，因为lua_state已经被清除
		m_StoryScript = nullptr;

		// 清除场景
		if (m_Scene)
		{
			m_LayeredSceneManager->ClearAll();
		}

		m_DialogueSystem->Reset();
	}

	void GalGameEngine::Wait(float duration)
	{
		if (m_DialogueSystem->IsFastForward())
			return;

		m_Wait.Helper.SetDuration(std::max(duration, 0.f));
		m_Wait.Helper.Reset();
		m_Wait.IsWait = true;
	}

	void GalGameEngine::CaptureSceneImage()
	{
		VGFX::TexturePixels scenePixels;
		if (m_EngineContext->GetViewport()->GetViewportTexture()->ReadPixels(scenePixels))
		{
			TextureConverter::SaveAsJPG(scenePixels, "D:/test.jpg");
		}
		
	}

	void GalGameEngine::OnMainSceneChanged(const EngineEvent& evt)
	{
		Reset();
		m_DialogueSystem->ClearDialogList();

		// 先设置场景
		m_Scene = dynamic_cast<Scene*>(evt.Scene);
		m_RenderPipeline->SetScene(m_Scene);

		m_DialogueSystem->FastForward(false);
		m_DialogueSystem->AutoDialogue(false);

		// 加载脚本
		if (GetSceneManager()->IsPlayMode())
		{
			auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();

			view.each([this](GalGameEngineComponent& com) { // flecs::entity argument is optional
				m_StoryScript = com.script;
				});

			if (m_StoryScript)
			{
				m_StoryScript->Run(this);
			}
		}

		if (m_StoryScript == nullptr)
		{
			m_IsEngineEnable = false;
		}
		else
		{
			m_IsEngineEnable = true;
		}
	}

	void GalGameEngine::CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext)
	{
		// 初始化对话系统
		m_DialogueSystem = CreateRef<DialogueSystem>();
		m_DialogueSystem->InitialiseDataModel(uiContext);

		// 初始化分层场景管理器
		m_LayeredSceneManager = CreateRef<LayeredSceneManager>();

		// 初始化渲染管线
		m_RenderPipeline = CreateRef<RenderPipeline>();
		m_RenderPipeline->Initialize(context);

		// 初始化存档系统
		m_ArchiveSystem = CreateRef<ArchiveSystem>();
		m_ArchiveSystem->Initialise();
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

	bool GalGameEngine::SceneTransition(const Ref<ISceneTransition>& transition)
	{
		TransitionManager::GetInstance()->StartTransition(transition);

		//m_RenderPipeline->CaptureScene();
		//
		//auto* actor = m_Scene->CreateActor();
		//actor->SetLabel("SceneTransition");
		//auto* com = actor->AddComponent<SceneTransitionComponent>();
		//com->transition = transition;

		return true;
	}

	bool GalGameEngine::TransitionCommand(const String& layer, const String& cmd)
	{
		if (m_DialogueSystem->IsFastForward())
			return true;

		auto tLayer = layer;
		if (tLayer == "背景")
		{
			tLayer = "Background";
		}
		else if (tLayer == "前景")
		{
			tLayer = "Foreground";
		}
		else if (tLayer == "屏幕")
		{
			tLayer = "Screen";
		}

		return TransitionManager::GetInstance()->StartTransitionWithCommand(tLayer, cmd);
	}

	bool GalGameEngine::TransitionCommandWithCustomImage(const String& layer, const String& imagePath,
		const String& cmd)
	{
		if (m_DialogueSystem->IsFastForward())
			return true;

		auto tLayer = layer;
		if (tLayer == "背景")
		{
			tLayer = "Background";
		}
		else if (tLayer == "前景")
		{
			tLayer = "Foreground";
		}
		else if (tLayer == "屏幕")
		{
			tLayer = "Screen";
		}

		String path = Core::GetAssetsPathVFS() + imagePath;

		return TransitionManager::GetInstance()->StartCustomImageTransitionWithCommand(tLayer, path, cmd);
	}

	bool GalGameEngine::PreLoadResource(const String& path)
	{
		if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".bmp") || path.ends_with(".tga"))
		{
			std::thread thread([path, this]()
				{
					String resPath = Core::GetAssetsPathVFS() + path;
					AssetManager::GetInstance()->LoadAsset<TextureAsset>(resPath);
				});

			thread.detach();
		}

		return true;
	}

	GameActor* GalGameEngine::CreateSpriteInline(const std::string& path)
	{
		// 读取纹理资产
		auto tex = LoadObject<Texture2D>(path);
		if (tex == nullptr)
		{
			H_LOG_WARN("加载图片失败: %s", path.c_str());
			return nullptr;
		}

		// 创建精灵角色
		auto* actor = m_Scene->CreateActor();
		actor->SetLabel(path);
		//actor->SetVisible(false);

		// 添加必要组件
		actor->AddComponent<AnimationScriptComponent>();
		actor->AddComponent<SpriteRendererComponent>()->sprite = Sprite::Create(tex, tex->Size());

		return actor;
	}

	GalSprite* GalGameEngine::AddSprite(GameActor* actor, const std::string& layer, const std::string& path)
	{
		if (actor == nullptr)
			return nullptr;

		// 创建GalGame的图片类
		GalSprite* sprite = new GalSprite(layer, path);
		sprite->m_Actor = actor;

		auto* spriteCom = actor->GetComponent<SpriteRendererComponent>();
		if (spriteCom)
		{
			spriteCom->pipelineIndex = static_cast<uint>(RenderPipelineIndex::GalGamePipeline);

			// 设置屏幕专属渲染管线
			if (layer == "Screen")
			{
				spriteCom->pipelineIndex = static_cast<uint>(RenderPipelineIndex::ScreenPipeline);
			}
		}

		// 对齐底部
		sprite->AlignBottom();

		// 添加到管理器
		m_LayeredSceneManager->AddSprite(sprite);

		return sprite;
	}

	GalSprite* GalGameEngine::ShowSprite(const std::string& layer, const std::string& path)
	{
		String resPath = Core::GetAssetsPathVFS() + path;
		GameActor* actor = CreateSpriteInline(resPath);
		return AddSprite(actor, layer, resPath);
	}

	GalSprite* GalGameEngine::ShowColor(const std::string& layer, const float4& color)
	{
		String resPath = EngineResource::GetDefaultSpriteTexturePath();
		GameActor* actor = CreateSpriteInline(resPath);

		GalSprite* sprite = AddSprite(actor, layer, resPath);

		// 设置全屏颜色
		actor->GetComponent<TransformComponent>()->scale = float3(999999, 999999, 1);
		actor->GetComponent<SpriteRendererComponent>()->color = color;

		return sprite;
	}

	GalAudio* GalGameEngine::PlayAudio(const std::string& layer, const std::string& path)
	{
		String resPath = Core::GetAssetsPathVFS() + path;

		// 读取音频资产
		auto audioClip = LoadObject<AudioClip>(resPath);
		if (audioClip == nullptr)
		{
			H_LOG_WARN("Failed to load audio: %s", resPath.c_str());
			return nullptr;
		}

		// 创建音频角色
		auto* actor = m_Scene->CreateActor();
		actor->SetLabel(resPath);

		// 添加音频源组件
		auto* audioSource = actor->AddComponent<AudioSourceComponent>();
		audioSource->audioClip = audioClip;
		audioSource->Play();

		// 创建GalGame的音频类
		GalAudio* audio = new GalAudio(layer, path);
		audio->m_Actor = actor;

		// 添加到管理器
		m_LayeredSceneManager->AddAudio(audio);

		return audio;
	}

	GalCharacter* GalGameEngine::CreateCharacter(const String& name)
	{
		auto character = CreateRef<GalCharacter>(name);
		m_Characters.push_back(character);

		return character.get();
	}

	void GalGameEngine::HideAllCharacterSprite()
	{
		for (auto& character: m_Characters)
		{
			character->HideFigure();
		}
	}

	bool GalGameEngine::RemoveSprite(GalSprite* sprite)
	{
		return m_LayeredSceneManager->RemoveSprite(sprite);
	}

	bool GalGameEngine::RemoveAudio(GalAudio* audio)
	{
		return m_LayeredSceneManager->RemoveAudio(audio);
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

	void GalGameEngine::OnRender()
	{

	}

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

	void GalGameEngine::AddUpdateCallback(const std::function<void()>& callback)
	{
		m_UpdateCallback.push_back(callback);
	}

	void GalGameEngine::OnUpdate(float deltaTime)
	{
		if (m_IsEngineEnable == false)
			return;

		// 回调
		for (auto& callback : m_UpdateCallback)
			callback();
		m_UpdateCallback.clear();

		// 更新对话系统
		GetDialogueSystem()->Update();

		// 更新存档系统
		SaveArchive archive;
		if (m_StoryScript)
			archive.scriptPath = m_StoryScript->GetResourcePath();	//脚本路径
		archive.line = m_DialogueSystem->GetCurrentDialogLine();	//对话当前行
		archive.description = m_DialogueSystem->GetCurrentDialogText();
		m_ArchiveSystem->UpdateSaveArchive(archive);

		// 更新变换脚本
		//SpriteTransformScriptManager::GetInstance()->Update(m_Scene);

		// 更新等待状态
		if (m_Wait.IsWait)
		{
			m_Wait.Helper.Update();
			if (m_Wait.Helper.IsFinished())
			{
				m_Wait.IsWait = false;
				StoryScriptLuaInterface::Continue();
			}
		}
	}
}
