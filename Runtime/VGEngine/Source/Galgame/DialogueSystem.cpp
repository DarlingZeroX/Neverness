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

#include "Galgame/DialogueSystem.h"
#include <codecvt>

#include "Engine/Manager/SceneManager.h"
#include "Galgame/GameEngineCore.h"
#include "Galgame/LayeredSceneManager.h"
#include "Galgame/GameLua.h"
#include "Galgame/StoryScriptLuaInterface.h"
#include "HCore/Include/System/HSystemTimer.h"
#include "Render/TransitionManager.h"

namespace VisionGal::GalGame
{
	TypingEffect::TypingEffect(std::string& outtext)
		:display_text(outtext)
	{
	}

	void TypingEffect::StartTyping(const std::string& full_text)
	{
		target_text = full_text;
		display_text.clear();
		last_update_time = std::chrono::high_resolution_clock::now();
		is_typing = true;

		// 调用打印回调
		for (auto& callback:m_TypingCallbacks)
		{
			sol::protected_function_result res = callback("", false);
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}

		current_char_pos = 0;
	}

	void TypingEffect::Update()
	{
		if (!is_typing)
			return;

		auto current_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = current_time - last_update_time;

		if (elapsed.count() >= typing_delay && current_char_pos < target_text.length()) {
			// 获取下一个完整的UTF-8字符
			auto [next_char, char_length] = GetNextUtf8Char(target_text, current_char_pos);

			// 添加完整字符
			display_text += next_char;

			// 更新位置
			current_char_pos += char_length;

			// 更新计时器
			last_update_time = current_time;

			// 检查是否完成
			if (current_char_pos >= target_text.length())
				is_typing = false;

			// 调用打印回调
			try {
				for (auto& callback : m_TypingCallbacks)
				{
					sol::protected_function_result res = callback(next_char, true);
					if (!res.valid()) {
						sol::error err = res;
						H_LOG_ERROR("%s", err.what());
					}
				}
			}
			catch (const sol::error& err) 
			{
				H_LOG_ERROR("%s", err.what());
				m_TypingCallbacks.clear();
			}
			catch (...)
			{
				m_TypingCallbacks.clear();
			}

			//std::cout << display_text.length() << std::endl;
		}
	}

	std::wstring TypingEffect::Utf8ToWString(const std::string& utf8_str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(utf8_str);
	}

	std::string TypingEffect::WStringToUtf8(const std::wstring& wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}

	std::pair<std::string, size_t> TypingEffect::GetNextUtf8Char(const std::string& utf8_str, size_t start_pos)
	{
		if (start_pos >= utf8_str.length())
			return { "", 0 };

		// UTF-8编码规则：
		// 1字节: 0xxxxxxx
		// 2字节: 110xxxxx 10xxxxxx
		// 3字节: 1110xxxx 10xxxxxx 10xxxxxx
		// 4字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

		unsigned char first_byte = static_cast<unsigned char>(utf8_str[start_pos]);
		size_t char_length = 1;

		// 确定字符长度
		if ((first_byte & 0x80) == 0) {
			// 1字节字符 (ASCII)
			char_length = 1;
		}
		else if ((first_byte & 0xE0) == 0xC0) {
			// 2字节字符
			char_length = 2;
		}
		else if ((first_byte & 0xF0) == 0xE0) {
			// 3字节字符 (常见于中文)
			char_length = 3;
		}
		else if ((first_byte & 0xF8) == 0xF0) {
			// 4字节字符 (表情符号等)
			char_length = 4;
		}

		// 确保不会超出字符串边界
		if (start_pos + char_length > utf8_str.length())
			char_length = utf8_str.length() - start_pos;

		return { utf8_str.substr(start_pos, char_length), char_length };
	}

	void TypingEffect::FinishTyping()
	{
		display_text = target_text;
		current_char_pos = target_text.length();
		is_typing = false;

		// 调用打印回调
		for (auto& callback:m_TypingCallbacks)
		{
			sol::protected_function_result res = callback(target_text, is_typing);
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}
	}

	float TypingEffect::GetTypingDelay()
	{
		return typing_delay;
	}

	void TypingEffect::SetTypingDelay(float delay)
	{
		typing_delay = delay;
	}

	void TypingEffect::AddTypingCallback(sol::function callback)
	{
		if (SceneManager::Get()->IsPlayMode() == false)
			return;

		m_TypingCallbacks.push_back(callback);
	}

	void TypingEffect::ClearAllTypingCallbacks()
	{
		//if (SceneManager::Get()->IsPlayMode() == false)
		//	return;

		m_TypingCallbacks.clear();
	}

	DialogueSystem::DialogueSystem()
		:m_TypingEffect(m_DialogText)
	{
	}

	DialogueSystem::~DialogueSystem()
	{
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

		if (m_EnableTyping)
		{
			m_TypingEffect.StartTyping(text);
		}
		else
		{
			m_DialogText = text;
		}

		m_CurrentDialogLine += 1;
		m_DialogList.push_back({ character, text });

		//H_LOG_INFO("[Character: %s]: %s", character.c_str(), text.c_str());
	}

	void DialogueSystem::EnableTyping(bool enable)
	{
		m_EnableTyping = enable;
	}

	void DialogueSystem::FinishTyping()
	{
		m_TypingEffect.FinishTyping();
	}

	bool DialogueSystem::IsTypingText()
	{
		if (!m_EnableTyping)
			return false;

		return m_TypingEffect.IsTyping();
	}

	void DialogueSystem::ContinueDialogue()
	{
		StoryScriptLuaInterface::Continue();
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
		return m_CurrentDialogLine;
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
		return m_DialogName;
	}

	String DialogueSystem::GetCurrentDialogText()
	{
		return m_DialogText;
	}

	void DialogueSystem::AutoDialogue(bool enable)
	{
		m_EnableAutoDialogue = enable;
		m_WaitingForNextAuto = false;
	}

	bool DialogueSystem::IsAutoDialogue() const
	{
		return m_EnableAutoDialogue;
	}

	void DialogueSystem::FastForward(bool enable)
	{
		m_EnableFastForward = enable;

		if (m_EnableFastForward && m_TypingEffect.IsTyping())
		{
			m_TypingEffect.FinishTyping();
		}
	}

	bool DialogueSystem::IsFastForward() const
	{
		return m_EnableFastForward;
	}

	void DialogueSystem::SetFastForwardDelay(float delay)
	{
		m_FastForwardDelay = delay;
	}

	float DialogueSystem::GetFastForwardDelay() const
	{
		return m_FastForwardDelay;
	}

	bool DialogueSystem::IsVoicing()
	{
		LayeredSceneManager* sceneManager = dynamic_cast<LayeredSceneManager*>(GameEngineCore::GetCurrentEngine()->GetLayeredSceneManager());

		SceneAudioManager::AudioLayer* voiceLayer = sceneManager->GetAudioLayer("Voice");

		return voiceLayer->IsPlayFinished() == false;
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
		m_CurrentDialogLine = 0;
		//FastForward(false);
		//AutoDialogue(false);
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
		m_CurrentDialogLine = 0;
		m_DialogName.clear();
		m_DialogText.clear();
		m_TypingEffect.FinishTyping();
		m_WaitingForNextAuto = false;
		m_FastForwardWaitingForNextAuto = false;
		m_EnableFastForward = false;
		m_EnableAutoDialogue = false;
	}

	void DialogueSystem::ProcessFastForward()
	{
		// 没有开启快进
		if (!m_EnableFastForward)
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
		if (elapsed.count() >= m_FastForwardDelay)
		{
			ContinueDialogue();
			m_FastForwardWaitingForNextAuto = false; // 重置计时器状态
		}
	}

	void DialogueSystem::ProcessAutoDialogue()
	{
		// 没有开启自动对话
		if (!m_EnableAutoDialogue)
			return;

		// 还在打字
		if (IsTypingText())
			return;

		// 播放语音中
		if (IsVoicing())
			return;

		// 开启快进时，不处理自动对话
		if (m_EnableFastForward)
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
		if (elapsed.count() >= m_AutoDelay)
		{
			ContinueDialogue();
			m_WaitingForNextAuto = false; // 重置计时器状态
		}
	}
}
