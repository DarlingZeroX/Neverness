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

#pragma once
#include "../Interface/GalgameInterface.h"
#include "../Core/GalGameContext.h"
#include "TypingEffect.h"
#include <RmlUi/Core.h>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialogueSystem: public IDialogueSystem
	{
	public:
		DialogueSystem();
		DialogueSystem(const DialogueSystem&) = delete;
		DialogueSystem& operator=(const DialogueSystem&) = delete;
		DialogueSystem(DialogueSystem&&) noexcept = default;
		DialogueSystem& operator=(DialogueSystem&&) noexcept = default;
		~DialogueSystem() override;

		void Initialize(const Ref<GalGameContext>& ctx);
		bool InitialiseDataModel(Rml::Context* context);

		void CharacterSay(const String& character, const String& text) override;	// 角色说话
		void EnableTyping(bool enable = true) override;			// 开启打字机效果
		void FinishTyping() override;							// 完成打字效果
		bool IsTypingText() override;							// 是否正在打字
		void ContinueDialogue();								// 继续对话，通常用于脚本中调用
		float GetTypingDelay();									// 获取打字延迟
		void SetTypingDelay(float delay);						// 设置打字延迟

		uint GetCurrentDialogLine() const override;				// 获取当前对话从开始是第几个对话
		uint GetDialogNumber() const override;					// 获取对话数量
		String GetDialogCharacter(uint index) override;			// 获取对话角色
		String GetDialogText(uint index) override;				// 获取对话文本
		String GetCurrentCharacter() override;					// 获取当前对话角色
		String GetCurrentDialogText() override;					// 获取当前对话文本

		void AutoDialogue(bool enable) override;				// 开启自动对话
		bool IsAutoDialogue() const override;					// 是否已经开启自动对话

		void FastForward(bool enable) override;					// 开启快进功能
		bool IsFastForward() const override;					// 是否开启快进功能
		void SetFastForwardDelay(float delay) override;			// 设置快进间隔
		float GetFastForwardDelay() const override;				// 获取快进间隔

		bool IsVoicing();										// 是否正在播放语音

		void AddTypingCallback(sol::function callback);
		void ClearAllTypingCallbacks();

		// 跳到对话
		void JumpToDialog(const std::string& text);

		void Reset();
		void Clear();
		void Update() override;
		void ClearDialogList();
	private:
		void ProcessFastForward();
		void ProcessAutoDialogue();

		Ref<GalGameContext> m_GalGameContext;
		Rml::DataModelHandle m_ModelHandle;
		// 打字效果
		TypingEffect m_TypingEffect;

		// 对话
		std::string m_DialogName;
		std::string m_DialogText;
		struct Dialog
		{
			String character;
			String text;
		};
		std::vector<Dialog> m_DialogList;

		// 快进
		std::chrono::high_resolution_clock::time_point m_FastForwardTimerStart;
		bool m_FastForwardWaitingForNextAuto = false;

		// 自动对话
		std::chrono::high_resolution_clock::time_point m_AutoTimerStart;
		bool m_WaitingForNextAuto = false;
	};
}
