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

#include "DialogueSystem/TypingEffect.h"
#include "NNEngineLegacy/Include/Engine/Manager/SceneManager.h"
#include <codecvt>

namespace VisionGal::GalGame
{
	TypingEffect::TypingEffect(std::string& outtext)
		:m_DisplayText(outtext)
	{
	}

	void TypingEffect::ApplyImmediateText(const std::string& full_text)
	{
		m_TargetText = full_text;
		m_DisplayText = full_text;
		m_IsTyping = false;
		m_CurrentCharPos = full_text.length();
	}

	void TypingEffect::StartTyping(const std::string& full_text)
	{
		m_TargetText = full_text;
		m_DisplayText.clear();
		m_LastUpdateTime = std::chrono::high_resolution_clock::now();
		m_IsTyping = true;

		// 调用打印回调
		for (auto& callback : m_TypingCallbacks)
		{
			sol::protected_function_result res = callback("", false);
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}

		m_CurrentCharPos = 0;
	}

	void TypingEffect::Update()
	{
		if (!m_IsTyping)
			return;

		auto current_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = current_time - m_LastUpdateTime;

		if (elapsed.count() >= m_TypingDelay && m_CurrentCharPos < m_TargetText.length()) {
			// 获取下一个完整的UTF-8字符
			auto [next_char, char_length] = GetNextUtf8Char(m_TargetText, m_CurrentCharPos);

			// 添加完整字符
			m_DisplayText += next_char;

			// 更新位置
			m_CurrentCharPos += char_length;

			// 更新计时器
			m_LastUpdateTime = current_time;

			// 检查是否完成
			if (m_CurrentCharPos >= m_TargetText.length())
				m_IsTyping = false;

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
		m_DisplayText = m_TargetText;
		m_CurrentCharPos = m_TargetText.length();
		m_IsTyping = false;

		// 调用打印回调
		for (auto& callback : m_TypingCallbacks)
		{
			sol::protected_function_result res = callback(m_TargetText, m_IsTyping);
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}
	}

	float TypingEffect::GetTypingDelay()
	{
		return m_TypingDelay;
	}

	void TypingEffect::SetTypingDelay(float delay)
	{
		m_TypingDelay = delay;
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

}
