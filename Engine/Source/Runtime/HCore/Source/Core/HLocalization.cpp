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

#include "pch.h"
#include "Core/HLocalization.h"

namespace Horizon
{
	std::string GetTranslateText(const std::string& text)
	{
		return HLocalizationManager::GetInstance()->Translate(text);
	}

	std::string_view HLocalDictionaryBase::Translate(std::string_view view)
	{
		return view;
	}

	void HLocalizationManager::SetLanguage(HLocalLanguageType lang)
	{
		m_LanguageType = lang;
		m_TranslateDictionary = &m_LanguageMap[lang];
	}

	bool HLocalizationManager::SetLanguageByCode(const std::string& code)
	{
		for (size_t i = 0; i < m_LanguageTypeCode.size(); ++i)
		{
			if (m_LanguageTypeCode[i] == code)
			{
				SetLanguage(static_cast<HLocalLanguageType>(i));
				return true;
			}
		}

		return false;
	}

	std::string HLocalizationManager::Translate(const std::string& text) const
	{
		auto result = m_TranslateDictionary->find(text);
		if (result != m_TranslateDictionary->end())
		{
			return result->second;
		}

		return text;
	}

	std::string HLocalizationManager::Translate(HLocalLanguageType type, const std::string& text) const
	{
		auto languageDictionary = m_LanguageMap.find(type);
		if (languageDictionary != m_LanguageMap.end())
		{
			auto result = languageDictionary->second.find(text);
			if (result != languageDictionary->second.end())
			{
				return result->second;
			}
		}

		return text;
	}

	std::string HLocalizationManager::GetCurrentLanguageCode() const
	{
		// 确保语言类型在范围内
		if (static_cast<size_t>(m_LanguageType) < m_LanguageTypeCode.size())
		{
			return m_LanguageTypeCode[static_cast<size_t>(m_LanguageType)];
		}

		return "EN-US";
	}

	void HLocalizationManager::MergeLanguageDictionary(HLocalLanguageType type, HLanguageDictionary& dictionary)
	{
		auto& language = m_LanguageMap[type];
		language.merge(dictionary);
	}

	HLocalizationManager::HLocalizationManager()
	{
		m_LanguageTypeCode = {
			"EN-US",
			"ZH-CN"
		};

		SetLanguage(HLocalLanguageType::EN_US);
	}

	HLocalizationManager::~HLocalizationManager()
	{

	}

	HLocalizationManager* HLocalizationManager::GetInstance()
	{
		static HLocalizationManager s_Instance;
		return &s_Instance;
	}

	HText::HText(const char* view)
		:
		m_view(view)
	{
	}

	HText::HText(std::string_view view)
		:
		m_view(view)
	{
	}

	HText::HText(const std::string& str)
		:
		m_view(str)
	{
	}

	HText& HText::operator=(const char* view)
	{
		m_view = view;
		return *this;
	}

	HText& HText::operator=(const std::string_view view)
	{
		m_view = view;
		return *this;
	}

	HText& HText::operator=(const std::string& str)
	{
		m_view = str;
		return *this;
	}

	const char* HText::text() const
	{
		return GetTranslateText(m_view).c_str();
	}
}