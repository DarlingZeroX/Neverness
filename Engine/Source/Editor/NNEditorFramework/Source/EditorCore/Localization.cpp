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

#include "EditorCore/Localization.h"

#include "EditorCore/EditorCore.h"
#include "NNRuntimeCore/Include/Core/VFS.h"

namespace NN::Editor
{
	bool EditorLoadLanguage(const std::string& code)
	{
		Runtime::IStringStreamVFS zhcnLocalization;
		zhcnLocalization.Open(EditorCore::GetEditorResourcePathVFS() + "localization/ZH_CN.txt");

		if (zhcnLocalization.IsOpen())
		{
			auto& stream = zhcnLocalization.GetStream();

			std::string line;
			NN::Core::HLanguageDictionary zhcnDictionary;

			while (std::getline(stream, line))
			{
				auto pos = line.find('=');
				if (pos != std::string::npos)
				{
					auto count = line.size() - (pos + 1) - 1;
					auto value = line.substr(pos + 1, count);
					zhcnDictionary[line.substr(0, pos)] = value;
				}

			}

			NN::Core::HLocalizationManager::GetInstance()->MergeLanguageDictionary(NN::Core::HLocalLanguageType::ZH_CN, zhcnDictionary);

		}

		if (code == "ZH-CN")
		{
			NN::Core::HLocalizationManager::GetInstance()->SetLanguage(NN::Core::HLocalLanguageType::ZH_CN);
			return true;
		}

		if (code == "EN-US")
		{
			NN::Core::HLocalizationManager::GetInstance()->SetLanguage(NN::Core::HLocalLanguageType::EN_US);
			return true;
		}

		return false;
	}

	EditorText::EditorText(const std::string& title)
	{
		m_Text = NN::Core::GetTranslateText(title);
	}


	EditorText::EditorText(const std::string& title, const std::string& icon)
	{
		m_Text = icon;
		m_Text += " ";
		m_Text += NN::Core::GetTranslateText(title);
	}

	EditorText::EditorText(const std::string& title, const std::string& icon, const std::string& id)
	{
		m_Text = icon;
		m_Text += " ";
		m_Text += NN::Core::GetTranslateText(title);
		m_Text += "##";
		m_Text += id;
	}

	const std::string& EditorText::GetText()
	{
		return m_Text;
	}

	const char* EditorText::c_str()
	{
		return m_Text.c_str();
	}
}
