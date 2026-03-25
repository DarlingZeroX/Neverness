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
#include <chrono>
#include "sol/function.hpp"

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
		bool IsTyping() const { return m_IsTyping; }
		void FinishTyping();
		float GetTypingDelay();									// 获取打字延迟
		void SetTypingDelay(float delay);						// 设置打字延迟

		void AddTypingCallback(sol::function callback);
		void ClearAllTypingCallbacks();
	private:
		std::string& m_DisplayText;  // 当前显示的文本
		std::string m_TargetText;   // 完整目标文本
		float m_TypingDelay = 0.02f; // 每个字符之间的延迟（秒）
		bool m_IsTyping = false;
		std::chrono::high_resolution_clock::time_point m_LastUpdateTime;
		size_t m_CurrentCharPos = 0; // 当前字符位置（按字节）

		std::vector<sol::function> m_TypingCallbacks;	// 打字回调列表
	};
}
