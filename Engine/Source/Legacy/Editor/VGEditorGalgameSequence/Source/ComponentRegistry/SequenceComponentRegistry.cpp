/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ComponentRegistry/SequenceComponentRegistry.h"

#include <NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

#include <algorithm>

namespace VisionGal::Editor
{
	namespace
	{
		struct DefaultCategoryRow
		{
			const char* Name;
			const char* TabIcon;
		};

		const DefaultCategoryRow kDefaultPaletteCategories[] = {
			{ u8"常规演出", ICON_FA_FILM },
			{ u8"舞台对象控制", ICON_FA_CUBE },
			{ u8"特殊演出", ICON_FA_SPARKLES },
			{ u8"场景与分支", ICON_FA_CODE_BRANCH },
			{ u8"赏鉴", ICON_FA_BOOK_OPEN },
			{ u8"游戏控制", ICON_FA_GAMEPAD },
		};
	}

	void SequenceComponentRegistry::Register(SequenceComponentMetadata metadata)
	{
		const std::string key = metadata.TypeNameID;
		m_byType[key] = std::move(metadata);
		if (std::find(m_registrationOrder.begin(), m_registrationOrder.end(), key) == m_registrationOrder.end())
			m_registrationOrder.push_back(key);
	}

	void SequenceComponentRegistry::Unregister(const std::string& typeNameID)
	{
		m_byType.erase(typeNameID);
		const auto it = std::find(m_registrationOrder.begin(), m_registrationOrder.end(), typeNameID);
		if (it != m_registrationOrder.end())
			m_registrationOrder.erase(it);
	}

	const SequenceComponentMetadata* SequenceComponentRegistry::Find(const std::string& typeNameID) const
	{
		const auto it = m_byType.find(typeNameID);
		if (it == m_byType.end())
			return nullptr;
		return &it->second;
	}

	std::vector<SequenceComponentMetadata> SequenceComponentRegistry::EnumerateOrdered() const
	{
		std::vector<SequenceComponentMetadata> out;
		out.reserve(m_registrationOrder.size());
		for (const std::string& id : m_registrationOrder)
		{
			const auto it = m_byType.find(id);
			if (it != m_byType.end())
				out.push_back(it->second);
		}
		std::stable_sort(out.begin(), out.end(), [](const SequenceComponentMetadata& a, const SequenceComponentMetadata& b) {
			if (a.Priority != b.Priority)
				return a.Priority < b.Priority;
			return a.TypeNameID < b.TypeNameID;
		});
		return out;
	}

	std::vector<CategoryData> SequenceComponentRegistry::BuildPaletteCategories() const
	{
		std::vector<CategoryData> categories;
		const std::size_t defaultCount = sizeof(kDefaultPaletteCategories) / sizeof(kDefaultPaletteCategories[0]);
		categories.resize(defaultCount);
		for (std::size_t i = 0; i < defaultCount; ++i)
		{
			categories[i].Name = kDefaultPaletteCategories[i].Name;
			categories[i].TabIcon = kDefaultPaletteCategories[i].TabIcon;
		}

		for (const SequenceComponentMetadata& meta : EnumerateOrdered())
		{
			std::size_t targetIndex = 0;
			bool matched = false;
			for (std::size_t i = 0; i < categories.size(); ++i)
			{
				if (categories[i].Name == meta.Category)
				{
					targetIndex = i;
					matched = true;
					break;
				}
			}
			if (!matched)
			{
				CategoryData extra;
				extra.Name = meta.Category;
				extra.TabIcon = meta.Icon.empty() ? ICON_FA_CUBE : meta.Icon;
				extra.Items.push_back(meta);
				categories.push_back(std::move(extra));
			}
			else
				categories[targetIndex].Items.push_back(meta);
		}

		return categories;
	}
}
