// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <string>
#include <unordered_map>
#include "../CoreModuleDefinitions.h"

namespace NN::Core
{
	enum class HLocalLanguageType
	{
		EN_US = 0,
		ZH_CN = 1,
	};

	using HLanguageDictionary = std::unordered_map<std::string, std::string>;

	CORE_MODULE_API std::string GetTranslateText(const std::string& text);

	struct HLocalDictionaryBase
	{
		virtual ~HLocalDictionaryBase() = default;

		virtual std::string_view Translate(std::string_view view);
	};
	 
	class CORE_MODULE_API HLocalizationManager
	{
	public:
		HLocalizationManager();
		HLocalizationManager(const HLocalizationManager&) = delete;
		HLocalizationManager& operator=(const HLocalizationManager&) = delete;
		HLocalizationManager(HLocalizationManager&&) noexcept = default;
		HLocalizationManager& operator=(HLocalizationManager&&) noexcept = default;
		~HLocalizationManager();

		static HLocalizationManager* GetInstance();

		// 设置本地语言
		void SetLanguage(HLocalLanguageType lang);

		bool SetLanguageByCode(const std::string& code);

		// 翻译成本地语言
		std::string Translate(const std::string& text) const;

		// 翻译成当前语言
		std::string Translate(HLocalLanguageType type, const std::string& text) const;

		// 获取当前语言
		HLocalLanguageType GetCurrentLanguage() const { return m_LanguageType; }

		std::string GetCurrentLanguageCode() const;

		const std::vector<std::string>& GetLanguageTypeCode() const { return m_LanguageTypeCode; }

		void MergeLanguageDictionary(HLocalLanguageType type ,HLanguageDictionary& dictionary);
	private:
		HLocalLanguageType m_LanguageType = HLocalLanguageType::EN_US;
		std::unordered_map<HLocalLanguageType, HLanguageDictionary> m_LanguageMap;
		HLanguageDictionary* m_TranslateDictionary = nullptr;
		std::vector<std::string> m_LanguageTypeCode;
	};

	class CORE_MODULE_API HText
	{
	public:
		HText(const char* view);
		HText(std::string_view view);
		HText(const std::string& str);
		HText(const HText&) = default;
		HText& operator=(const HText&) = default;
		HText(HText&&) noexcept = default;
		HText& operator=(HText&&) noexcept = default;
		HText& operator=(const char* view);
		HText& operator=(const std::string_view view);
		HText& operator=(const std::string& str);

		const char* text() const;
	private:
		std::string m_view;
	};
}



