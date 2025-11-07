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
#include "Interface.h"
#include "../EngineConfig.h"
#include <chrono>
#include <RmlUi/Core.h>
 
namespace VisionGal::GalGame
{
	class TypingEffect
	{
	public:
		TypingEffect(std::string& outtext);
		TypingEffect(const TypingEffect&) = delete;
		TypingEffect& operator=(const TypingEffect&) = delete;
		TypingEffect(TypingEffect&&) noexcept = default;
		TypingEffect& operator=(TypingEffect&&) noexcept = default;
		~TypingEffect() = default;

		void StartTyping(const std::string& full_text);
		void Update();

		std::wstring Utf8ToWString(const std::string& utf8_str);			// 辅助函数：将UTF-8字符串转换为UTF-16 wstring
		std::string WStringToUtf8(const std::wstring& wstr);			// 辅助函数：将UTF-16 wstring转换为UTF-8字符串

		std::pair<std::string, size_t> GetNextUtf8Char(const std::string& utf8_str, size_t start_pos);			// 辅助函数：获取UTF-8字符串中的下一个完整字符及其长度
		bool IsTyping() const { return is_typing; }
		void FinishTyping();
		float GetTypingDelay();									// 获取打字延迟
		void SetTypingDelay(float delay);						// 设置打字延迟
	private:
		std::string& display_text;  // 当前显示的文本
		std::string target_text;   // 完整目标文本
		float typing_delay = 0.02f; // 每个字符之间的延迟（秒）
		bool is_typing = false;
		std::chrono::high_resolution_clock::time_point last_update_time;

		size_t current_char_pos = 0; // 当前字符位置（按字节）
	};

	class VG_ENGINE_API DialogueSystem: public IDialogueSystem
	{
	public:
		DialogueSystem();
		DialogueSystem(const DialogueSystem&) = delete;
		DialogueSystem& operator=(const DialogueSystem&) = delete;
		DialogueSystem(DialogueSystem&&) noexcept = default;
		DialogueSystem& operator=(DialogueSystem&&) noexcept = default;
		~DialogueSystem() override;

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

		// 跳到对话
		void JumpToDialog(const std::string& text);

		void Reset();
		void Update() override;
		void ClearDialogList();
	private:
		std::string m_DialogName;
		std::string m_DialogText;

		Rml::DataModelHandle m_ModelHandle;

		// 打字效果
		TypingEffect m_TypingEffect;
		bool m_EnableTyping = true;

		// 对话
		struct Dialog
		{
			String character;
			String text;
		};
		uint m_CurrentDialogLine = 0;
		std::vector<Dialog> m_DialogList;

		// 快进
		bool m_EnableFastForward = false;
		float m_FastForwardDelay = 0.f;
		std::chrono::high_resolution_clock::time_point m_FastForwardTimerStart;
		bool m_FastForwardWaitingForNextAuto = false;

		// 自动对话
		bool m_EnableAutoDialogue = false;
		float m_AutoDelay = 2.0f;
		std::chrono::high_resolution_clock::time_point m_AutoTimerStart;
		bool m_WaitingForNextAuto = false;
	};
}