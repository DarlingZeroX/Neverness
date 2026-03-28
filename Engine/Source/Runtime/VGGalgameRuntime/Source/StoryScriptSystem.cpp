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

#include "StoryScriptSystem.h"
#include "VGGalgameCore/Include/Components.h"
#include "VGCore/Include/Core/EventBus.h"

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
		//StoryScriptLuaInterface::ResetStoryScript();

		// 加载脚本
		//auto storyScript = LuaStoryScript::LoadFromFile(path);
		auto storyScript = StoryScriptExecutorFactory::GetInstance().LoadStoryScriptExecutorFromFile("lua", path);

		if (storyScript == nullptr)
		{
			H_LOG_WARN("加载剧情脚本失败: %s", path.c_str());
			return false;
		}

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

	void StoryScriptSystem::DoChoice(const std::string& id, const std::vector<std::string>& options)
	{
		// 如果正在加载存档，则直接从存档数据中获取选择结果
		if (m_GalGameContext->runtimeState.isCurrentLoadingArchive)
		{
			std::string choice = m_GalGameContext->archiveData->GetChoicesNamespace()->GetVariable<std::string>(id);

			// 获取索引
			int choiceIndex = 0;
			for (;choiceIndex < options.size(); choiceIndex++)
			{
				if (options[choiceIndex] == choice)
					break;
			}

			// 延迟到存档读取循环中执行,因为这里还在调用协程中，直接继续协程会出现不需要继续协程
			m_ArchiveLuaReadCallback.push_back([this,id,options,choiceIndex] ()
				{
					OnChoiceSelected(id, options, choiceIndex);
				});

			return;
		}

		GalGameUIEvent evt;
		evt.ChoiceID = id;
		evt.ChoiceOptions = options;
		evt.EventType = GalGameUIEvent::Type::ShowChoiceUI;
		m_GalGameContext->uiEventBus.OnUIEvent.Invoke(evt);
	}

	void StoryScriptSystem::DoInput(const std::string& id, const std::string& title, const std::string& button)
	{
		// 如果正在加载存档，则直接从存档数据中获取选择结果
		if (m_GalGameContext->runtimeState.isCurrentLoadingArchive)
		{
			std::string text = m_GalGameContext->archiveData->GetInputNamespace()->GetVariable<std::string>(id);
			//OnInputSubmitted(id, text);
			// 延迟到存档读取循环中执行,因为这里还在协程中，直接继续协程会出现不需要继续协程
			m_ArchiveLuaReadCallback.push_back([this, id, text]()
				{
					OnInputSubmitted(id, text);
				});
			return;
		}

		GalGameUIEvent evt;
		evt.InputID = id;
		evt.InputTitle = title;
		evt.InputButtonText = button;
		evt.EventType = GalGameUIEvent::Type::ShowInputUI;
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
		// 设置状态
		m_GalGameContext->runtimeState.enableFastForward = true;
		m_GalGameContext->runtimeState.isCurrentLoadingArchive = true;
		// 读取存档数据
		m_GalGameContext->archiveData = archive.archiveData;

		if (!LoadStoryScript(archive.scriptPath))
		{
			m_GalGameContext->runtimeState.enableFastForward = false;
			m_GalGameContext->runtimeState.isCurrentLoadingArchive = false;
			return false;
		}

		while (archive.line > m_GalGameContext->runtimeState.currentDialogLine)
		{
			ContinueDialogue();

			// 执行存档读取时的Lua回调
			for (auto& callback : m_ArchiveLuaReadCallback)
				callback();
			m_ArchiveLuaReadCallback.clear();
		}

		m_GalGameContext->runtimeState.enableFastForward = false;
		m_GalGameContext->runtimeState.isCurrentLoadingArchive = false;
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
					OnChoiceSelected(evt.ChoiceID, evt.ChoiceOptions, evt.CurrentChoiceIndex);
					break;
				case GalGameUIEvent::Type::InputSubmitted:
					OnInputSubmitted(evt.InputID, evt.CurrentInputText);
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

	void StoryScriptSystem::OnChoiceSelected(
		const std::string& id,
		const std::vector<std::string>& options,
		int currentChoice
	)
	{
		// 保存选择结果到存档数据
		m_GalGameContext->archiveData->GetChoicesNamespace()->SetVariable(id, options[currentChoice]);

		H_ASSERT_NOT_NULL(m_StoryScript);
		if (m_StoryScript != nullptr)
		{
			m_StoryScript->OnChoiceSelected(id, options, currentChoice);
		}
	}

	void StoryScriptSystem::OnInputSubmitted(const std::string& id, const std::string& text)
	{
		// 保存输入结果到存档数据
		m_GalGameContext->archiveData->GetInputNamespace()->SetVariable(id, text);

		H_ASSERT_NOT_NULL(m_StoryScript);
		if (m_StoryScript != nullptr)
		{
			m_StoryScript->OnInputSubmitted(id, text);
		}
	}

	void StoryScriptSystem::ContinueDialogue()
	{
		H_ASSERT_NOT_NULL(m_StoryScript);
		if (m_StoryScript != nullptr)
		{
			m_StoryScript->ContinueDialogue();
		}
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
				if (m_StoryScript != nullptr)
				{
					m_StoryScript->ContinueDialogue();
				}
				//StoryScriptLuaInterface::Continue();
			}
		}
	}
}
