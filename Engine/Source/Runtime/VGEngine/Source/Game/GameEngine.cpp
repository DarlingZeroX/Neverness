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

#include "Game/GameEngine.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGCore/Include/Core/Input.h"
#include "VGUI/Interface/UISystem.h"
#include "Engine/Manager.h"
#include "Scene/Components.h"
#include "Galgame/GalGameEngine.h"
#include "Render/TransitionManager.h"
#include "CoreLua.h"

namespace VisionGal
{
	IUISystem* CoreGameEngineContext::GetUISystem()
	{
		return uiSystem;
	}

	Horizon::SDL3::OpenGLWindow* CoreGameEngineContext::GetWindow()
	{
		return window;
	}
	 
	Viewport* CoreGameEngineContext::GetViewport()
	{
		return viewport;
	}

	void CoreGameEngineContext::AddBeforeRenderCallback(const String& callbackID, const std::function<void(OpenGL::RenderTarget2D*)>& callback)
	{
		beforeRenderCallbacks[callbackID] = callback;
	}

	void CoreGameEngineContext::ExecuteBeforeRenderCallbacks(OpenGL::RenderTarget2D* rt)
	{
		for (auto& pair : beforeRenderCallbacks)
		{
			pair.second(rt);
		}
	}

	void CoreGameEngineContext::AddAfterRenderCallback(const String& callbackID, const std::function<void(OpenGL::RenderTarget2D*)>& callback)
	{
		afterRenderCallbacks[callbackID] = callback;
	}

	void CoreGameEngineContext::ExecuteAfterRenderCallbacks(OpenGL::RenderTarget2D* rt)
	{
		for (auto& pair : afterRenderCallbacks)
		{
			pair.second(rt);
		}
	}

	CoreGameEngine::CoreGameEngine()
	{
		m_RenderEngine = MakeRef<CoreRenderEngine>();

		// 订阅引擎事件
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

	void CoreGameEngine::SetRenderFinalResultToScreen(bool enable)
	{
		m_RenderEngine->SetRenderFinalResultToScreen(enable);
	}

	void CoreGameEngine::OnMainSceneChanged(const EngineEvent& evt)
	{
		// 先关闭前一个场景之前播放的资源
		if (evt.PrevScene != nullptr)
		{
			// 视频
			{
				auto view = evt.PrevScene->GetWorld()->view<VideoPlayerComponent>();
				view.each([this](VideoPlayerComponent& com) { // flecs::entity argument is optional
					if (com.videoClip)
					{
						com.Stop();
					}
					});
			}

			// 音频
			{
				auto view = evt.PrevScene->GetWorld()->view<AudioSourceComponent>();
				view.each([this](AudioSourceComponent& com) { // flecs::entity argument is optional
					if (com.audioClip)
					{
						com.Stop();
					}
					});
			}
		}

		// 加载当前场景的资源
		if (GetSceneManager()->IsPlayMode())
		{
			// 脚本
			{
				auto view = GetSceneManager()->GetCurrentRunningScene()->GetWorld()->view<ScriptComponent>();
				view.each([this](ScriptComponent& com) { // flecs::entity argument is optional
					for (auto& script : com.scripts)
					{
						if (script != nullptr)
						{
							IGameActor* actor = dynamic_cast<IGameActor*>(com.GetOwner());
							script->Awake(actor);
							script->Start(actor);
						}
					}
					});
			}

			// 视频
			{
				auto view = evt.Scene->GetWorld()->view<VideoPlayerComponent>();
				view.each([this](VideoPlayerComponent& com) { // flecs::entity argument is optional
					if (com.videoClip && com.playOnAwake)
					{
						com.Play();
					}
					});
			}

			// 音频
			{
				auto view = evt.Scene->GetWorld()->view<AudioSourceComponent>();
				view.each([this](AudioSourceComponent& com) { // flecs::entity argument is optional
					if (com.audioClip && com.playOnAwake)
					{
						com.Play();
					}
					});
			}
		}

		UISystem::Get()->CloseAllDocuments();

		// UI	不管是否处在场景播放模式，都需要加载UI
		{
			auto view = evt.Scene->GetWorld()->view<RmlUIDocumentComponent>();
			view.each([this](RmlUIDocumentComponent& com) { // flecs::entity argument is optional
				if (com.document)
				{
					// 先加载UI
					com.document = UISystem::Get()->LoadUIDocument(com.document->GetResourcePath());
					// 再显示UI
					UISystem::Get()->ShowUIDocument(com.document.get());
				}

				});
		}
	}

	void CoreGameEngine::OnUpdate(float deltaTime)
	{
		auto* scene = GetSceneManager()->GetCurrentRunningScene();

		if (scene == nullptr)
			return;

		CoreLua::Update();

		////////////////////////// 下面是输入的更新 //////////////////////////
		{
			// 更新输入
			Input::Get()->Update();
		}

		////////////////////////// 下面是数据的更新 //////////////////////////
		{
			// 更新子引擎
			for (auto& subEngine : m_SubGameEngines)
			{
				subEngine->OnUpdate(deltaTime);
			}

			// 更新动画脚本
			{
				auto view = scene->GetWorld()->view<AnimationScriptComponent>();

				view.each([this](AnimationScriptComponent& com) { // flecs::entity argument is optional
					for (auto& script : com.scripts)
					{
						script->OnUpdate(com.GetOwner());
					}
					});
			}

			// 更新所有脚本
			if (GetSceneManager()->IsPlayMode())
			{
				auto view = scene->GetWorld()->view<ScriptComponent>();
				view.each([this, deltaTime](ScriptComponent& com) { // flecs::entity argument is optional
					for (auto& script : com.scripts)
					{
						if (script != nullptr)
						{
							IGameActor* actor = dynamic_cast<IGameActor*>(com.GetOwner());
							script->Update(actor, deltaTime);
						}
					}
					});
			}

			// 更新所有矩阵,放在最后更新，因为Actor脚本，剧情脚本，子系统可能会修改Transform
			{
				auto view = scene->GetWorld()->view<TransformComponent>();
				view.each([this](TransformComponent& com) { // flecs::entity argument is optional
					com.Update();
					});
			}
		}

		////////////////////////// 下面是渲染相关的更新 //////////////////////////
		{
			// 更新转场管理器
			TransitionManager::GetInstance()->Update();

			// 更新渲染引擎
			m_RenderEngine->OnUpdate(deltaTime);
		}

		////////////////////////// 最后是场景相关的更新 //////////////////////////
		{
			SceneManager::Get()->Update(deltaTime);
		}

		//if (Input::GetKeyDown("B"))
		//{
		//	H_LOG_INFO("GetKeyDown B");
		//}
	}

	void CoreGameEngine::OnRender()
	{
		// 子引擎渲染
		for (auto& subEngine : m_SubGameEngines)
		{
			subEngine->OnRender();
		}

		// 核心渲染引擎渲染
		m_RenderEngine->OnRender();
	}

	void CoreGameEngine::Initialize(Horizon::SDL3::OpenGLWindow* window)
	{
		// 创建视口
		m_Viewport = GetViewportManager()->NewViewport({ window->WindowWidth(), window->WindowHeight() });
		GetViewportManager()->SetMainViewport(m_Viewport);

		// 设置上下文
		m_EngineContext.viewport = m_Viewport;
		m_EngineContext.window = window;
		m_EngineContext.uiSystem = UISystem::Get();

		// 初始化渲染引擎
		//m_RenderEngine->Initialize(window, m_Viewport);
		m_RenderEngine->Initialize(&m_EngineContext);

		// 初始化输入
		Input::Get()->AttachWindow(window);
		Input::Get()->AttachViewport(m_Viewport);

		// 放在最后创建GalGame子引擎，因为有些系统还没初始化，比如UI系统
		auto galgameEngine = MakeRef<GalGame::GalGameEngine>();
		galgameEngine->Initialize(&m_EngineContext);
		m_SubGameEngines.push_back(galgameEngine);

		// 初始化 Core Lua
		CoreLua::Initialize();
	}
}
