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

#include "DialogueSystem/DialogueSystem.h"
#include "VGEngine/Include/Render/TransitionManager.h"

namespace VisionGal::GalGame
{
	DialogueSystem::DialogueSystem()
		:m_TypingEffect(m_DialogText)
	{
	}

	DialogueSystem::~DialogueSystem()
	{
	}

	void DialogueSystem::Initialize(const Ref<GalGameContext>& ctx)
	{
		m_GalGameContext = ctx;
	}

	bool DialogueSystem::InitialiseDataModel(Rml::Context* context)
	{
		Rml::DataModelConstructor constructor = context->CreateDataModel("dialog");
		if (!constructor)
			return false;

		constructor.Bind("dialog_name", &m_DialogName);
		constructor.Bind("dialog_text", &m_DialogText);

		m_ModelHandle = constructor.GetModelHandle();

		return true;
	}

	void DialogueSystem::CharacterSay(const String& character, const String& text)
	{
		m_DialogName = character;
		m_GalGameContext->runtimeState.currentDialogCharacter = character;

		if (m_GalGameContext->runtimeState.enableTyping)
		{
			m_TypingEffect.StartTyping(text);
		}
		else
		{
			m_DialogText = text;
		}

		m_GalGameContext->runtimeState.currentDialogText = text;

		//m_CurrentDialogLine += 1;
		m_GalGameContext->runtimeState.currentDialogLine += 1;
		m_DialogList.push_back({ character, text });

		//H_LOG_INFO("[Character: %s]: %s", character.c_str(), text.c_str());
	}

	void DialogueSystem::EnableTyping(bool enable)
	{
		m_GalGameContext->runtimeState.enableTyping = enable;
	}

	void DialogueSystem::FinishTyping()
	{
		m_TypingEffect.FinishTyping();
	}

	bool DialogueSystem::IsTypingText()
	{
		if (!m_GalGameContext->runtimeState.enableTyping)
			return false;

		return m_TypingEffect.IsTyping();
	}

	void DialogueSystem::ContinueDialogue()
	{
		GalGameScriptExecuteEvent evt;
		evt.EventType = GalGameScriptExecuteEventType::ContinueExecute;
		m_GalGameContext->engineEventBus.OnStoryScriptExecuteEvent.Invoke(evt);
	}

	float DialogueSystem::GetTypingDelay()
	{
		return m_TypingEffect.GetTypingDelay();
	}

	void DialogueSystem::SetTypingDelay(float delay)
	{
		m_TypingEffect.SetTypingDelay(delay);
	}

	uint DialogueSystem::GetCurrentDialogLine() const
	{
		return m_GalGameContext->runtimeState.currentDialogLine;
	}

	uint DialogueSystem::GetDialogNumber() const
	{
		return m_DialogList.size();
	}

	static const char* DialogueSystemEmptyString = "";

	String DialogueSystem::GetDialogCharacter(uint index)
	{
		if (index < m_DialogList.size())
		{
			return m_DialogList[index].character.c_str();
		}

		return DialogueSystemEmptyString;
	}

	String DialogueSystem::GetDialogText(uint index)
	{
		if (index < m_DialogList.size())
		{
			return m_DialogList[index].text.c_str();
		}

		return DialogueSystemEmptyString;
	}

	String DialogueSystem::GetCurrentCharacter()
	{
		return m_GalGameContext->runtimeState.currentDialogCharacter;
	}

	String DialogueSystem::GetCurrentDialogText()
	{
		return m_GalGameContext->runtimeState.currentDialogText;
	}

	void DialogueSystem::AutoDialogue(bool enable)
	{
		//m_EnableAutoDialogue = enable;
		m_GalGameContext->runtimeState.enableAutoDialogue = enable;
		m_WaitingForNextAuto = false;
	}

	bool DialogueSystem::IsAutoDialogue() const
	{
		return m_GalGameContext->runtimeState.enableAutoDialogue;
	}

	void DialogueSystem::FastForward(bool enable)
	{
		//m_EnableFastForward = enable;
		m_GalGameContext->runtimeState.enableFastForward = enable;

		//if (m_EnableFastForward && m_TypingEffect.IsTyping())
		if (m_GalGameContext->runtimeState.enableFastForward && m_TypingEffect.IsTyping())
		{
			m_TypingEffect.FinishTyping();
		}
	}

	bool DialogueSystem::IsFastForward() const
	{
		//return m_EnableFastForward;
		return m_GalGameContext->runtimeState.enableFastForward;
	}

	void DialogueSystem::SetFastForwardDelay(float delay)
	{
		//m_FastForwardDelay = delay;
		m_GalGameContext->runtimeState.fastForwardDelay = delay;
	}

	float DialogueSystem::GetFastForwardDelay() const
	{
		//return m_FastForwardDelay;
		return m_GalGameContext->runtimeState.fastForwardDelay;
	}

	bool DialogueSystem::IsVoicing()
	{
		return m_GalGameContext->runtimeState.IsVoicing;
	}

	void DialogueSystem::AddTypingCallback(sol::function callback)
	{
		m_TypingEffect.AddTypingCallback(callback);
	}

	void DialogueSystem::ClearAllTypingCallbacks()
	{
		m_TypingEffect.ClearAllTypingCallbacks();
	}

	void DialogueSystem::JumpToDialog(const std::string& text)
	{
		bool loop = true;
		while (loop)
		{
			ContinueDialogue();
			if (GetCurrentDialogText() == text)
			{
				loop = false;
			}
		}
	}

	void DialogueSystem::Reset()
	{
		//m_CurrentDialogLine = 0;
		//FastForward(false);
		//AutoDialogue(false);
	}

	void DialogueSystem::Clear()
	{
		ClearDialogList();
		FastForward(false);
		AutoDialogue(false);
	}

	void DialogueSystem::Update()
	{
		m_TypingEffect.Update();

		// 处理快进
		ProcessFastForward();

		// 处理自动对话
		ProcessAutoDialogue();

		m_ModelHandle.DirtyAllVariables();
	}

	void DialogueSystem::ClearDialogList()
	{
		m_DialogList.clear();
		m_TypingEffect.FinishTyping();
		m_WaitingForNextAuto = false;
		m_FastForwardWaitingForNextAuto = false;
		//m_EnableFastForward = false;
		//m_EnableAutoDialogue = false;
		m_GalGameContext->runtimeState.enableAutoDialogue = false;
		m_GalGameContext->runtimeState.enableFastForward = false;
	}

	void DialogueSystem::ProcessFastForward()
	{
		// 没有开启快进
		if (!IsFastForward())
			return;

		m_TypingEffect.FinishTyping();

		// 已经打字结束，准备自动切换
		if (!m_FastForwardWaitingForNextAuto)
		{
			// 启动计时器
			m_FastForwardTimerStart = std::chrono::high_resolution_clock::now();
			m_FastForwardWaitingForNextAuto = true;
			return;
		}

		// 计时器到达，继续对话
		auto current_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = current_time - m_FastForwardTimerStart;
		//if (elapsed.count() >= m_FastForwardDelay)
		if (elapsed.count() >= m_GalGameContext->runtimeState.fastForwardDelay)
		{
			ContinueDialogue();
			m_FastForwardWaitingForNextAuto = false; // 重置计时器状态
		}
	}

	void DialogueSystem::ProcessAutoDialogue()
	{
		// 没有开启自动对话
		if (!IsAutoDialogue())
			return;

		// 还在打字
		if (IsTypingText())
			return;

		// 播放语音中
		if (IsVoicing())
			return;

		// 开启快进时，不处理自动对话
		if (IsFastForward())
		{
			AutoDialogue(false);
			return;
		}

		// 正在转场中
		if (TransitionManager::GetInstance()->IsTransitioning())
			return;

		// 已经打字结束，准备自动切换
		if (!m_WaitingForNextAuto)
		{
			// 启动计时器
			m_AutoTimerStart = std::chrono::high_resolution_clock::now();
			m_WaitingForNextAuto = true;
			return;
		}

		// 计时器到达，继续对话
		auto current_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = current_time - m_AutoTimerStart;
		//if (elapsed.count() >= m_AutoDelay)
		if (elapsed.count() >= m_GalGameContext->runtimeState.autoDialogueDelay)
		{
			ContinueDialogue();
			m_WaitingForNextAuto = false; // 重置计时器状态
		}
	}
}
