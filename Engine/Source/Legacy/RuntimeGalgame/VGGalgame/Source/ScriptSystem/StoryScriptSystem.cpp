/*
 * StoryScriptSystem 实现（VGGalgame / ScriptSystem）
 *
 * 中文要点：
 * - `LoadStoryScript`：**GalRuntimeScriptLoader**（**Registry → Factory**）构造 **IStoryScriptExecutor** → `Run`；
 *   若 **`IScriptRuntime::TryCreateStoryExecution`** 返回非空则直接持有内核 **IStoryExecutionInstance**，否则 **`StoryExecutionInstance`** 包装。
 * - `LoadArchive`：置位「读档快进」后反复 `ContinueDialogue` 直至对白行号追上存档记录；期间
 *   选择/输入分支通过 `m_ArchiveLuaReadCallback` 延迟到循环内执行，避免在 Lua 协程栈内重入。
 */

#include "ScriptSystem/StoryScriptSystem.h"

#include "ScriptSystem/GalAssetTypeScriptRuntime.h"
#include "ScriptSystem/GalRuntimeScriptLoader.h"
#include "NNRuntimeAsset/Include/GalGameAsset.h"
#include "VGGalgameSequenceRuntime/Include/Asset/Asset.h"
#include "VGGalgameCore/Include/Components.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "VGGalgameRuntimeCore/Include/SaveArchive.h"
#include "VGGalgameRuntimeCore/Include/GalGameRuntimeState.h"
#include "VGGalgameRuntimeCore/Include/ArchiveDataContainer.h"

namespace VisionGal::GalGame
{
	bool StoryScriptSystem::ReloadStoryScript()
	{
		AddUpdateCallback([this]()
			{
				LoadSceneStoryScript(m_Scene);
			});

		return true;
	}

	bool StoryScriptSystem::LoadStoryScript(const String& path)
	{
		Ref<IStoryScriptExecutor> storyScript = m_ScriptLoader.LoadExecutorForPath(path);

		if (storyScript == nullptr)
		{
			H_LOG_WARN("加载剧情脚本失败: %s", path.c_str());
			return false;
		}

		m_GalGameContext->runtimeState.dialogue.currentDialogLine = 0;

		{
			GalGameScriptEvent evt;
			evt.scriptPath = path;
			evt.EventType = GalGameScriptEventType::OnScriptStartLoad;
			m_GalGameContext->engineEventBus.OnStoryScriptEvent.Invoke(evt);
		}

		if (m_SubsystemBus == nullptr)
		{
			H_LOG_ERROR("StoryScriptSystem: SubsystemBus 未注入，无法 Run");
			return false;
		}

		bool result = storyScript->Run(m_SubsystemBus, m_GalGameContext.get());

		if (result == false)
			return false;

		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();
		view.each([this, path](GalGameEngineComponent& com) {
			com.scriptPath = path;
			});

		m_StoryScript = storyScript;

		m_GalGameContext->runtimeState.currentScriptPath = path;

		{
			GalGameScriptEvent evt;
			evt.scriptPath = path;
			evt.EventType = GalGameScriptEventType::OnScriptFinishedLoad;
			m_GalGameContext->engineEventBus.OnStoryScriptEvent.Invoke(evt);
		}

		if (IScriptRuntime* rt = m_RuntimeRegistry.FindRuntimeForPath(path))
		{
			if (Ref<IStoryExecutionInstance> direct = rt->TryCreateStoryExecution(path, m_StoryScript.get()))
			{
				m_ExecutionInstance = std::move(direct);
				return true;
			}
		}

		m_ExecutionInstance = MakeRef<StoryExecutionInstance>(m_StoryScript);
		return true;
	}

	bool StoryScriptSystem::LoadStoryScriptOnUpdate(const String& path)
	{
		AddUpdateCallback([this, path]()
			{
				LoadStoryScript(path);
			});
		return true;
	}

	IStoryExecutionInstance* StoryScriptSystem::GetExecutionInstance(unsigned int id) const
	{
		if (!m_ExecutionInstance)
			return nullptr;

		return m_ExecutionInstance.get();
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
		if (m_GalGameContext->runtimeState.playback.isCurrentLoadingArchive)
		{
			std::string choice = m_GalGameContext->archiveData->GetChoicesNamespace()->GetVariable<std::string>(id);

			int choiceIndex = 0;
			for (;choiceIndex < options.size(); choiceIndex++)
			{
				if (options[choiceIndex] == choice)
					break;
			}

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
		if (m_GalGameContext->runtimeState.playback.isCurrentLoadingArchive)
		{
			std::string text = m_GalGameContext->archiveData->GetInputNamespace()->GetVariable<std::string>(id);
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
		view.each([this,&scriptPath](GalGameEngineComponent& com) {
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
		m_Wait.Timer.SetDuration(std::max(duration, 0.f));
		m_Wait.Timer.Reset();
		m_Wait.IsWait = true;
	}

	bool StoryScriptSystem::LoadArchive(const SaveArchive& archive)
	{
		m_GalGameContext->runtimeState.playback.enableFastForward = true;
		m_GalGameContext->runtimeState.playback.isCurrentLoadingArchive = true;
		m_GalGameContext->archiveData = archive.archiveData;

		if (!LoadStoryScript(archive.scriptPath))
		{
			m_GalGameContext->runtimeState.playback.enableFastForward = false;
			m_GalGameContext->runtimeState.playback.isCurrentLoadingArchive = false;
			return false;
		}

		// 中文：将脚本推进到存档记录的对白行；每轮先 Continue 再刷延迟回调，避免协程内直接重入
		while (archive.line > m_GalGameContext->runtimeState.dialogue.currentDialogLine)
		{
			ContinueDialogue();

			for (auto& callback : m_ArchiveLuaReadCallback)
				callback();
			m_ArchiveLuaReadCallback.clear();
		}

		m_GalGameContext->runtimeState.playback.enableFastForward = false;
		m_GalGameContext->runtimeState.playback.isCurrentLoadingArchive = false;
	}

	void StoryScriptSystem::Initialise(const Ref<GalGameContext>& galCtx, IGameEngineContext* context, ISubsystemBus* subsystemBus)
	{
		m_GalGameContext = galCtx;
		m_GameEngineContext = context;
		m_SubsystemBus = subsystemBus;

		RegisterBuiltinScriptRuntimes();

		m_ScriptLoader.Attach(&m_RuntimeRegistry);

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

		if (m_StoryScript != nullptr)
		{
			m_StoryScript->Tick(0);
		}

		UpdateWaitState();
	}

	void StoryScriptSystem::RegisterBuiltinScriptRuntimes()
	{
		/// 中文：与 **GalGameLuaScriptModule::MountEngineRuntime** / **GalGameSequenceScriptModule::MountEngineRuntime** 注册的工厂类型 ID 保持一致。
		m_RuntimeRegistry.Clear();
		m_RuntimeRegistry.RegisterRuntime(MakeRef<GalAssetTypeScriptRuntime>(
			String(VisionGal::GLuaScriptAssetType{}.GetNameID()),
			String("GalGameLuaScript")));
		m_RuntimeRegistry.RegisterRuntime(MakeRef<GalAssetTypeScriptRuntime>(
			String(VisionGal::GalGame::SequenceScriptAssetType{}.GetNameID()),
			String("GalGameSequenceScript")));
	}

	void StoryScriptSystem::ResetExecutionPipeline()
	{
		m_UpdateCallback.clear();
		m_ArchiveLuaReadCallback.clear();
		m_Wait = {};
		m_StoryScript.reset();
		m_ExecutionInstance.reset();
		/// 中文：**GalGameRuntimeState** / **ArchiveDataContainer** 的清理由 **GalRuntimeCoordinator::ResetRuntime** 单点写入，
		/// 避免与协调器重复赋值及顺序倒置（先清场景再清状态 vs 先清状态）。
	}

	void StoryScriptSystem::OnChoiceSelected(
		const std::string& id,
		const std::vector<std::string>& options,
		int currentChoice
	)
	{
		m_GalGameContext->archiveData->GetChoicesNamespace()->SetVariable(id, options[currentChoice]);

		H_ASSERT_NOT_NULL(m_StoryScript);
		if (m_StoryScript != nullptr)
		{
			m_StoryScript->OnChoiceSelected(id, options, currentChoice);
		}
	}

	void StoryScriptSystem::OnInputSubmitted(const std::string& id, const std::string& text)
	{
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
			}
		}
	}
}
