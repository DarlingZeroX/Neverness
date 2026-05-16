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

#include "DialogueSystem/DialogueSystem.h"
#include "NNEngineLegacy/Include/Render/TransitionManager.h"

namespace VisionGal::GalGame
{
	DialogueSystem::DialogueSystem()
		: m_TypingRuntime(m_Presentation.MutableBoundDialogText())
	{
	}

	DialogueSystem::~DialogueSystem() = default;

	void DialogueSystem::Initialize(const Ref<GalGameContext>& ctx)
	{
		m_GalGameContext = ctx;
	}

	bool DialogueSystem::InitialiseDataModel(Rml::Context* context)
	{
		return m_Presentation.InitialiseDataModel(context);
	}

	void DialogueSystem::CharacterSay(const String& character, const String& text)
	{
		m_Presentation.SetBoundDialogName(character);
		m_LineRuntime.RecordUtterance(m_GalGameContext, character, text);
		m_TypingRuntime.StartLine(m_GalGameContext, text);
	}

	void DialogueSystem::EnableTyping(bool enable)
	{
		m_TypingRuntime.EnableTyping(m_GalGameContext, enable);
	}

	void DialogueSystem::FinishTyping()
	{
		m_TypingRuntime.FinishTyping();
	}

	bool DialogueSystem::IsTypingText()
	{
		return m_TypingRuntime.IsTypingText(m_GalGameContext);
	}

	void DialogueSystem::ContinueDialogue()
	{
		GalGameScriptExecuteEvent evt;
		evt.EventType = GalGameScriptExecuteEventType::ContinueExecute;
		if (m_GalGameContext)
			m_GalGameContext->engineEventBus.OnStoryScriptExecuteEvent.Invoke(evt);
	}

	float DialogueSystem::GetTypingDelay()
	{
		return m_TypingRuntime.GetTypingDelay();
	}

	void DialogueSystem::SetTypingDelay(float delay)
	{
		m_TypingRuntime.SetTypingDelay(delay);
	}

	uint DialogueSystem::GetCurrentDialogLine() const
	{
		return m_LineRuntime.GetCurrentDialogLine(m_GalGameContext);
	}

	uint DialogueSystem::GetDialogNumber() const
	{
		return m_LineRuntime.GetDialogNumber();
	}

	String DialogueSystem::GetDialogCharacter(uint index)
	{
		return m_LineRuntime.GetDialogCharacter(index);
	}

	String DialogueSystem::GetDialogText(uint index)
	{
		return m_LineRuntime.GetDialogText(index);
	}

	String DialogueSystem::GetCurrentCharacter()
	{
		return m_LineRuntime.GetCurrentCharacter(m_GalGameContext);
	}

	String DialogueSystem::GetCurrentDialogText()
	{
		return m_LineRuntime.GetCurrentDialogText(m_GalGameContext);
	}

	void DialogueSystem::AutoDialogue(bool enable)
	{
		m_PlaybackRuntime.AutoDialogue(m_GalGameContext, enable);
	}

	bool DialogueSystem::IsAutoDialogue() const
	{
		return m_PlaybackRuntime.IsAutoDialogue(m_GalGameContext);
	}

	void DialogueSystem::FastForward(bool enable)
	{
		m_PlaybackRuntime.FastForward(m_GalGameContext, enable, [this] { m_TypingRuntime.FinishTyping(); });
	}

	bool DialogueSystem::IsFastForward() const
	{
		return m_PlaybackRuntime.IsFastForward(m_GalGameContext);
	}

	void DialogueSystem::SetFastForwardDelay(float delay)
	{
		m_PlaybackRuntime.SetFastForwardDelay(m_GalGameContext, delay);
	}

	float DialogueSystem::GetFastForwardDelay() const
	{
		return m_PlaybackRuntime.GetFastForwardDelay(m_GalGameContext);
	}

	bool DialogueSystem::IsVoicing()
	{
		return m_PlaybackRuntime.IsVoicing(m_GalGameContext);
	}

	void DialogueSystem::AddTypingCallback(sol::function callback)
	{
		m_TypingRuntime.AddTypingCallback(std::move(callback));
	}

	void DialogueSystem::ClearAllTypingCallbacks()
	{
		m_TypingRuntime.ClearAllTypingCallbacks();
	}

	void DialogueSystem::JumpToDialog(const std::string& text)
	{
		bool loop = true;
		while (loop)
		{
			ContinueDialogue();
			if (GetCurrentDialogText() == text)
				loop = false;
		}
	}

	void DialogueSystem::Reset()
	{
	}

	void DialogueSystem::Clear()
	{
		ClearDialogList();
		FastForward(false);
		AutoDialogue(false);
	}

	void DialogueSystem::Update()
	{
		m_TypingRuntime.Update(m_GalGameContext);

		// 中文：快进开启时每帧强制完成打字（与原 DialogueSystem::ProcessFastForward 首段语义一致）。
		if (m_PlaybackRuntime.IsFastForward(m_GalGameContext))
			m_TypingRuntime.FinishTyping();

		m_PlaybackRuntime.Tick(
			m_GalGameContext,
			[this] { ContinueDialogue(); },
			[this] { return IsTypingText(); },
			[] {
				return TransitionManager::GetInstance()->IsTransitioning();
			});

		m_Presentation.MarkAllVariablesDirty();
	}

	void DialogueSystem::ClearDialogList()
	{
		m_LineRuntime.ClearList(m_GalGameContext);
		m_TypingRuntime.FinishTyping();
		m_PlaybackRuntime.ResetWaitState();
	}
}
