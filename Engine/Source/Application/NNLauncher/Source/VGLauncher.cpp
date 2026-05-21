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

#include "VGLauncherData.h"
#include "VGLauncher.h"
#include "MainWindow.h"
#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h>
#include "NNRuntimeVFS/Include/VFSService.h"
#include "NNRuntimePak/Include/VFSMount.h"
#include <NNRuntimeRmlui/Interface/UISystem.h>
#include <NNEditorFrameworkLegacy/Framework.h>

namespace VisionGal::Editor
{	
	VGLauncher::VGLauncher()
	{
	}
	 
	void VGLauncher::Initialize()
	{
		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		// 初始化启动器窗口数据
		auto editorName = "VisionGal Launcher";
		if (editorConfig.EditorLanguage == "ZH-CN")
			editorName = "VisionGal 启动器";

		auto windowWidth = 1280;
		auto windowHeight = 720;

		// 创建启动器窗口
		m_LauncherWindow = NN::MakeScope<VGWindow>();
		m_LauncherWindow->Initialize(editorName, windowWidth, windowHeight,true);

		// 添加ImGui Layer
		AddImGuiLayer();
		// 初始化编辑器UI
		InitializeEditorUI();
		// 初始化启动器面板
		InitializeLauncherPanels();
	}

	void VGLauncher::InitializeLauncherPanels()
	{
		auto* panelManager = NN::Editor::PanelManager::GetInstance();

		//editor->AddPanelWithID("EditorMenuBar", MakeRef<EditorMenuBar>());
		panelManager->AddPanelWithID("LauncherMainWindow", NN::MakeRef<VGLauncherMainWindow>());
	}

	void VGLauncher::InitializeEditorUI()
	{
		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		// 设置本地语言
		NN::Editor::EditorLoadLanguage(editorConfig.EditorLanguage);
		// 设置主题
		NN::Editor::EditorStyle::DarkTheme();
		// 初始化ImGuiEx
		ImGuiEx::Initialize();
	}

	void VGLauncher::AddImGuiLayer()
	{
		// 添加ImGui Layer
		m_LauncherWindow->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(m_LauncherWindow.get()));
		m_ImGuiOpenGL3Layer = std::make_unique<ImguiOpengl3Layer>(m_LauncherWindow.get(), m_LauncherWindow->GetContext());

		ImGuiIO& io = ImGui::GetIO();
		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		// 读取中文字体
		VFS::VFSService::SafeReadFileFromVFS(RuntimeCore::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const VFS::VFSService::DataRef& data) {
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig icons_config;
			icons_config.FontDataOwnedByAtlas = false;

			io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 20, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

			return 0;
			});

		// 读取图标字体
		VFS::VFSService::SafeReadFileFromVFS(RuntimeCore::GetEngineResourcePathVFS() + "fonts/fa-regular-400.ttf", [&](const VFS::VFSService::DataRef& data) {
			ImGuiIO& io = ImGui::GetIO();
			static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			icons_config.GlyphOffset = ImVec2(0, 2);
			icons_config.FontDataOwnedByAtlas = false;

			io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 20, &icons_config, icons_ranges);
			return 0;
			});

		// 构建字体图集
		io.Fonts->Build();
	}

	void VGLauncher::OnRender()
	{

	}

	void VGLauncher::OnUpdate(float deltaTime)
	{
		NN::Editor::PanelManager::GetInstance()->OnUpdate(deltaTime);
	}

	void VGLauncher::OnFixedUpdate()
	{
		NN::Editor::PanelManager::GetInstance()->OnFixedUpdate();
	}

	void VGLauncher::OnGUI()
	{
		m_ImGuiOpenGL3Layer->BeginFrame();
		NN::Editor::PanelManager::GetInstance()->OnGUI();
		ImGuiEx::Render();
		m_ImGuiOpenGL3Layer->EndFrame();
	}

	void VGLauncher::OnApplicationUpdate(float deltaTime)
	{
		OnFixedUpdate();
		OnUpdate(deltaTime);
		OnRender();
		OnGUI();

		m_LauncherWindow->SwapWindow();
	}

	int VGLauncher::ProcessEvent(const SDL_Event& event)
	{
		return m_LauncherWindow->ProcessEvent(event);
	}

	void VGLauncher::MakeCurrentRenderContext()
	{
		SDL_GL_MakeCurrent(m_LauncherWindow->GetSDLWindow(), m_LauncherWindow->GetContext());
	}
}
