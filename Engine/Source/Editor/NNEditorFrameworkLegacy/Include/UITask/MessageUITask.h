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
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeCore/Include/Core/RuntimeCore.h>
#include <NNPlatformCore/Interface/FileSystem/HFileSystem.h>

namespace NN::Editor
{
	class MessageUITask : public ImGuiEx::ImTaskInterface
	{
	public:
		MessageUITask() = default;
		MessageUITask(const MessageUITask&) = default;
		MessageUITask& operator=(const MessageUITask&) = default;
		MessageUITask(MessageUITask&&) noexcept = default;
		MessageUITask& operator=(MessageUITask&&) noexcept = default;
		MessageUITask(Runtime::String const& title, Runtime::String const& text);

		void SetChoices(const std::vector<Runtime::String>& choices);
		void SetCallback(const std::function<void(int)>& callback);
		void RenderUI(TaskContext& context) override;
	private:
		Runtime::String m_Title;
		Runtime::String m_Text;
		std::vector<Runtime::String> m_Choices;
		std::function<void(int)> m_Callback;
	};

}
