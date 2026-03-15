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
#include <VGCore/Include/Core/Window.h>
#include <VGEngine/Include/Game/GameEngine.h>
#include <VGEngine/Include/Engine/ImGuiLayer.h>
#include <VGCore/Interface/EngineInterface.h>

namespace VisionGal::Editor
{
	class VGLauncher: public IEngineApplication
	{
	public:
		VGLauncher();
		VGLauncher(const VGLauncher&) = delete;
		VGLauncher& operator=(const VGLauncher&) = delete;
		VGLauncher(VGLauncher&&) noexcept = default;
		VGLauncher& operator=(VGLauncher&&) noexcept = default;
		~VGLauncher() override = default;

		/// @brief 初始化启动器
		void Initialize();
		/// @brief 初始化启动器面板
		void InitializeLauncherPanels();

		/// @brief 添加一个应用层
		/// @param layer 应用层
		void AddApplicationLayer(IEngineApplicationLayer* layer) override {}
		/// @brief 启动器的主循环
		/// @param deltaTime 流逝的时间
		void OnApplicationUpdate(float deltaTime) override;

		/// @brief 处理事件
		int ProcessEvent(const SDL_Event& event) override;
		/// @brief 设置当前渲染上下文
		void MakeCurrentRenderContext() override;
	private:
		/// @brief 初始化编辑器的用户界面。
		void InitializeEditorUI();
		/// @brief 添加 ImGui Layer
		void AddImGuiLayer();
	private:
		void OnRender();
		void OnUpdate(float delta);
		void OnFixedUpdate();
		void OnGUI();
	private:
		Scope<VGWindow> m_LauncherWindow;	// 启动器窗口
		Scope<ImguiOpengl3Layer> m_ImGuiOpenGL3Layer;	// ImGui OpenGL3 渲染层
	};
}
