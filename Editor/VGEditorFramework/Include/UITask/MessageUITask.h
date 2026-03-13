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
#include <VGImgui/IncludeImGuiEx.h>
#include <VGEngine/Include/Core/Core.h>
#include <HCorePlatform/Interface/HFileSystem.h>

namespace VisionGal::Editor
{
	class MessageUITask : public ImGuiEx::ImTaskInterface
	{
	public:
		MessageUITask() = default;
		MessageUITask(const MessageUITask&) = default;
		MessageUITask& operator=(const MessageUITask&) = default;
		MessageUITask(MessageUITask&&) noexcept = default;
		MessageUITask& operator=(MessageUITask&&) noexcept = default;
		MessageUITask(String const& title, String const& text);

		void SetChoices(const std::vector<String>& choices);
		void SetCallback(const std::function<void(int)>& callback);
		void RenderUI(TaskContext& context) override;
	private:
		String m_Title;
		String m_Text;
		std::vector<String> m_Choices;
		std::function<void(int)> m_Callback;
	};

}
