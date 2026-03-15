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
#include "../EngineConfig.h"
#include "VGCore/Include/Core/Window.h"

namespace VisionGal
{
	struct VG_ENGINE_API ImguiOpengl3Layer
	{
		ImguiOpengl3Layer(Horizon::SDL3::Window* window, SDL_GLContext context);
		ImguiOpengl3Layer(const ImguiOpengl3Layer&) = delete;
		ImguiOpengl3Layer& operator=(const ImguiOpengl3Layer&) = delete;
		virtual ~ImguiOpengl3Layer();

		void BeginFrame();
		void EndFrame();
	private:
		void ImGuiBeginFrame();
		void ImGuiShutdown();
		void ImGuiRender();
	};

}