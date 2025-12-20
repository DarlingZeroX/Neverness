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

#include "Galgame/ScriptSystem/StoryScriptSystem.h"
#include "Galgame/Components.h"
#include "Core/EventBus.h"
#include "Galgame/Lua/StoryScriptLuaInterface.h"

namespace VisionGal::GalGame
{
	bool StoryScriptSystem::ReloadStoryScript()
	{
		AddUpdateCallback([this]()
			{
				LoadSceneStoryScript(m_Scene);
			});

		//AddUpdateCallback([this]()
		//	{
		//		// 加载脚本
		//
		//		m_StoryScript = LuaStoryScript::LoadFromFile(m_StoryScript->GetResourcePath());
		//
		//		if (m_StoryScript)
		//		{
		//			m_StoryScript->Run(m_GalGameEngine);
		//		}
		//	});

		return true;
	}

	bool StoryScriptSystem::LoadStoryScript(const String& path)
	{
		StoryScriptLuaInterface::ResetStoryScript();

		// 加载脚本
		auto storyScript = LuaStoryScript::LoadFromFile(path);

		if (storyScript == nullptr)
			return false;

		// 初始化当前对话行
		m_GalGameContext->runtimeState.currentDialogLine = 0;

		// 脚本开始加载事件
		{
			GalGameScriptEvent evt;
			evt.scriptPath = path;
			evt.EventType = GalGameScriptEventType::OnScriptStartLoad;
			m_GalGameContext->engineEventBus.OnStoryScriptEvent.Invoke(evt);
		}

		// 运行剧情脚本
		bool result = storyScript->Run(m_GalGameEngine);

		if (result == false)
			return false;

		// 把脚本设置到引擎组件
		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();
		view.each([this, path](GalGameEngineComponent& com) { // flecs::entity argument is optional
			com.scriptPath = path;
			});

		m_StoryScript = storyScript;

		// 设置脚本路径
		m_GalGameContext->runtimeState.currentScriptPath = path;

		// 脚本完成加载事件
		{
			GalGameScriptEvent evt;
			evt.scriptPath = path;
			evt.EventType = GalGameScriptEventType::OnScriptFinishedLoad;
			m_GalGameContext->engineEventBus.OnStoryScriptEvent.Invoke(evt);
		}
	}

	bool StoryScriptSystem::LoadStoryScriptOnUpdate(const String& path)
	{
		AddUpdateCallback([this, path]()
			{
				LoadStoryScript(path);
			});
		return true;
	}

	std::string StoryScriptSystem::GetCurrentStoryScriptPath()
	{
		if (m_StoryScript == nullptr)
			return {};

		return m_StoryScript->GetResourcePath();
	}

	std::filesystem::file_time_type StoryScriptSystem::GetScriptLastWriteTime() const
	{
		if (m_StoryScript == nullptr)
			return {};

		return m_StoryScript->GetScriptLastWriteTime();
	}

	void StoryScriptSystem::DoChoice(const std::string& name, const std::vector<std::string>& options)
	{
		GalGameUIEvent evt;
		evt.ChoiceName = name;
		evt.ChoiceOptions = options;
		evt.EventType = GalGameUIEvent::Type::ShowChoiceUI;
		m_GalGameContext->uiEventBus.OnUIEvent.Invoke(evt);
	}

	bool StoryScriptSystem::LoadSceneStoryScript(IScene* scene)
	{
		auto view = scene->GetWorld()->view<GalGameEngineComponent>();

		std::string scriptPath;
		view.each([this,&scriptPath](GalGameEngineComponent& com) { // flecs::entity argument is optional
			scriptPath = com.scriptPath;
			});

		return LoadStoryScript(scriptPath);
	}

	bool StoryScriptSystem::LoadSceneStoryScriptOnUpdate(IScene* scene)
	{
		AddUpdateCallback([this, scene]()
			{
				LoadSceneStoryScript(scene);
			});
		return true;
	}

	void StoryScriptSystem::Wait(float duration)
	{
		//if (m_DialogueSystem->IsFastForward())
		//	return;

		m_Wait.Timer.SetDuration(std::max(duration, 0.f));
		m_Wait.Timer.Reset();
		m_Wait.IsWait = true;
	}

	bool StoryScriptSystem::LoadArchive(const SaveArchive& archive)
	{
		m_GalGameContext->runtimeState.enableFastForward = true;

		if (!LoadStoryScript(archive.scriptPath))
		{
			m_GalGameContext->runtimeState.enableFastForward = false;
			return false;
		}

		while (archive.line > m_GalGameContext->runtimeState.currentDialogLine)
		{
			ContinueDialogue();
		}

		m_GalGameContext->runtimeState.enableFastForward = false;
	}

	void StoryScriptSystem::Initialise(const Ref<GalGameContext>& galCtx, IGameEngineContext* context)
	{
		m_GalGameContext = galCtx;
		m_GameEngineContext = context;

		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::MainSceneChanged:
					m_Scene = evt.Scene;
					break;
				}
			});

		m_GalGameContext->engineEventBus.OnStoryScriptExecuteEvent.Subscribe([this](const GalGameScriptExecuteEvent& evt)
			{
				switch (evt.EventType)
				{
				case GalGameScriptExecuteEventType::ContinueExecute:
					ContinueDialogue();
					break;
				}
			});

		m_GalGameContext->uiEventBus.OnUIEvent.Subscribe([this](const GalGameUIEvent& evt)
			{
				switch (evt.EventType)
				{
				case GalGameUIEvent::Type::ChoiceSelected:
					OnChoiceSelected("choice", evt.ChoiceOptions, evt.CurrentChoiceIndex);
					break;
				}
			});
	}

	void StoryScriptSystem::Update()
	{
		for (auto& callback : m_UpdateCallback)
			callback();
		m_UpdateCallback.clear();

		UpdateWaitState();
	}

	void StoryScriptSystem::SetEngine(IGalGameEngine* engine)
	{
		m_GalGameEngine = engine;
	}

	void StoryScriptSystem::OnChoiceSelected(const std::string& name, const std::vector<std::string>& options,
		int currentChoice)
	{
		StoryScriptLuaInterface::Continue(StoryScriptLuaInterface::ContinueType::String, 0, options[currentChoice]);
	}

	void StoryScriptSystem::ContinueDialogue()
	{
		StoryScriptLuaInterface::Continue();
	}

	void StoryScriptSystem::AddUpdateCallback(const std::function<void()>& callback)
	{
		m_UpdateCallback.push_back(callback);
	}

	void StoryScriptSystem::UpdateWaitState()
	{
		if (m_Wait.IsWait)
		{
			m_Wait.Timer.Update();
			if (m_Wait.Timer.IsFinished())
			{
				m_Wait.IsWait = false;
				StoryScriptLuaInterface::Continue();
			}
		}
	}
}
