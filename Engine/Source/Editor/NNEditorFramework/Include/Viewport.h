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
#include <vector>
#include <NNKernel/Interface/HConfig.h>
#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeCore/Include/Core/Viewport.h>

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API EditorViewport : public IEditorPanel
	{
	public:
		EditorViewport(Viewport* viewport);
		EditorViewport(const EditorViewport&) = delete;
		EditorViewport& operator=(const EditorViewport&) = delete;
		EditorViewport(EditorViewport&&) noexcept = default;
		EditorViewport& operator=(EditorViewport&&) noexcept = default;
		~EditorViewport() override = default;

		void OnGUI() override;

		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;
	private:
		Viewport* m_Viewport;
		std::vector<Ref<ISidebarComponent>> m_Components;
		bool m_IsOpen = true;
	};



}
