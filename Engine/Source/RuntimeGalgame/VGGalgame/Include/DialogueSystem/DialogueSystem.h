/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzeroox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * Phase 8：本类为 IDialogueSystem 的 **装配门面**，具体职责拆至：
 * - DialogueRmlPresentation（Rml 数据模型）
 * - DialogueLineRuntime（对白行 / 历史 / 与 Context 同步）
 * - DialogueTypingRuntime（打字机）
 * - DialoguePlaybackRuntime（自动 / 快进节拍）
 */

#pragma once
#include "../../VGGalgameConfig.h"
#include "VGGalgameCore/Interface/IGameSystem.h"
#include "VGGalgameCore/Include/GalGameContext.h"
#include "DialogueRmlPresentation.h"
#include "DialogueLineRuntime.h"
#include "DialogueTypingRuntime.h"
#include "DialoguePlaybackRuntime.h"
#include <RmlUi/Core.h>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialogueSystem : public IDialogueSystem
	{
	public:
		DialogueSystem();
		DialogueSystem(const DialogueSystem&) = delete;
		DialogueSystem& operator=(const DialogueSystem&) = delete;
		DialogueSystem(DialogueSystem&&) noexcept = default;
		DialogueSystem& operator=(DialogueSystem&&) noexcept = default;
		~DialogueSystem() override;

		void Initialize(const Ref<GalGameContext>& ctx);
		/// 中文：Rml 绑定入口；委托 DialogueRmlPresentation，Runtime 核心逻辑不依赖本调用是否成功。
		bool InitialiseDataModel(Rml::Context* context);

		void CharacterSay(const String& character, const String& text) override;
		void EnableTyping(bool enable = true) override;
		void FinishTyping() override;
		bool IsTypingText() override;
		void ContinueDialogue() override;
		float GetTypingDelay() override;
		void SetTypingDelay(float delay) override;

		uint GetCurrentDialogLine() const override;
		uint GetDialogNumber() const override;
		String GetDialogCharacter(uint index) override;
		String GetDialogText(uint index) override;
		String GetCurrentCharacter() override;
		String GetCurrentDialogText() override;

		void AutoDialogue(bool enable) override;
		bool IsAutoDialogue() const override;

		void FastForward(bool enable) override;
		bool IsFastForward() const override;
		void SetFastForwardDelay(float delay) override;
		float GetFastForwardDelay() const override;

		bool IsVoicing() override;

		void AddTypingCallback(sol::function callback) override;
		void ClearAllTypingCallbacks() override;

		void JumpToDialog(const std::string& text) override;

		void Reset() override;
		void Clear() override;
		void Update() override;
		void ClearDialogList() override;

	private:
		Ref<GalGameContext> m_GalGameContext;

		/// 中文：成员声明顺序即构造顺序；Typing 依赖 Presentation 提供的绑定字符串引用。
		DialogueRmlPresentation m_Presentation;
		DialogueTypingRuntime m_TypingRuntime;
		DialogueLineRuntime m_LineRuntime;
		DialoguePlaybackRuntime m_PlaybackRuntime;
	};
}
