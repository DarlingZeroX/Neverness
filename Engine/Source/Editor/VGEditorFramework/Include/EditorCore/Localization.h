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
#include "../Config.h"
#include <HCore/Include/Core/HLocalization.h>

namespace VisionGal::Editor
{
	VG_EDITOR_FRAMEWORK_API bool EditorLoadLanguage(const std::string& code);

	class VG_EDITOR_FRAMEWORK_API EditorText
	{
	public:
		EditorText(const std::string& title);
		EditorText(const std::string& title, const std::string& icon);
		EditorText(const std::string& title, const std::string& icon, const std::string& id);
		EditorText(const EditorText&) = default;
		EditorText& operator=(const EditorText&) = default;
		EditorText(EditorText&&) noexcept = default;
		EditorText& operator=(EditorText&&) noexcept = default;
		~EditorText() = default;

		const std::string& GetText();
		const char* c_str();
	private:
		std::string m_Text;
	};
}