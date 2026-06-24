/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "../../ApplicationExport.h"
#include "../Core/Window.h"
#include <memory>

// 前向声明
namespace NN::Runtime::Render { class INNRenderDevice; }

namespace NN::Runtime
{
	/**
	 * @brief Diligent ImGui 层。
	 * 使用 Diligent 的 ImGuiImplSDL3 后端（继承 ImGuiImplDiligent）。
	 * NNRuntimeImGui 编译 ImGui 核心（1.92.3），NNRuntimeDiligent 提供 Diligent 后端。
	 */
	struct NN_RUNTIME_APPLICATION_API ImguiDiligentLayer
	{
		ImguiDiligentLayer(NN::Core::SDL3::Window* window, Render::INNRenderDevice* device);
		ImguiDiligentLayer(const ImguiDiligentLayer&) = delete;
		ImguiDiligentLayer& operator=(const ImguiDiligentLayer&) = delete;
		virtual ~ImguiDiligentLayer();

		void BeginFrame();
		void EndFrame();
	private:
		void ImGuiInit();
		void ImGuiShutdown();
		void ImGuiRender();

		struct Impl;
		std::unique_ptr<Impl> m_Impl;
	};

}
