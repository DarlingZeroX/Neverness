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

#include "VGNodeGraphApp.h"
#include "MainWindow.h"
#include <HFileSystem/Interface/HFileSystem.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGImgui/Include/ImGuiLayer/SDL3Decorator.h>
#include <VGEditorCore/IncludeCore.h>
#include "VGEditorComponent/Framework.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGEngine/Include/Project/ProjectSettings.h"

namespace VisionGal::Editor
{
	VGNodeGraphApp::~VGNodeGraphApp()
	{
	}
	 
	void VGNodeGraphApp::Initialize()
	{
		auto& editorConfig = ProjectSettings::GetProjectSettings();

		auto editorName = editorConfig.Application.ApplicationName;
		auto windowWidth = editorConfig.Application.WindowWidth;
		auto windowHeight = editorConfig.Application.WindowHeight;

		m_ApplicationWindow = MakeRef<VGWindow>();
		m_ApplicationWindow->Initialize(editorName.c_str(), windowWidth, windowHeight,true);

		AddImguiLayer();
		InitializeEditorUI();
	}

	void VGNodeGraphApp::AddApplicationLayer(IEngineApplicationLayer* layer)
	{
	}

	void VGNodeGraphApp::InitializeEditorPanels()
	{
		auto* editor = PanelManager::GetInstance();

		//editor->AddPanelWithID("EditorMenuBar", MakeRef<EditorMenuBar>());
		editor->AddPanelWithID("VGNodeGraphAppMainWindow", MakeRef<VGNodeGraphMainWindow>());
	}

	void VGNodeGraphApp::InitializeEditorUI()
	{
		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		// 设置本地语言
		EditorLoadLanguage(editorConfig.EditorLanguage);

		// 设置主题
		EditorStyle::DarkTheme();

		// 创建Imgui的UI任务执行器
		ImGuiEx::ImTaskManager::CreateManager();
	}

	void VGNodeGraphApp::AddImguiLayer()
	{
		// 添加ImGui Layer
		m_ApplicationWindow->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(m_ApplicationWindow.get()));
		m_ImguiOpengl3Layer = std::make_unique<ImguiOpengl3Layer>(m_ApplicationWindow.get(), m_ApplicationWindow->GetContext());

		VFS::SafeReadFileFromVFS(Core::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const VFS::DataRef& data) {
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig icons_config;
			icons_config.FontDataOwnedByAtlas = false;

			//io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 17, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
			io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 20, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

			return 0;
			});

		ImGui::GetIO().Fonts->Build();
	}

	void VGNodeGraphApp::OnRender()
	{
	}

	void VGNodeGraphApp::OnUpdate(float deltaTime)
	{
		PanelManager::GetInstance()->OnUpdate(deltaTime);
	}

	void VGNodeGraphApp::OnFixedUpdate()
	{
		PanelManager::GetInstance()->OnFixedUpdate();
	}

	void VGNodeGraphApp::OnGUI()
	{
		m_ImguiOpengl3Layer->BeginFrame();
		PanelManager::GetInstance()->OnGUI();
		ImGuiEx::ImTaskManager::GetInstance().RenderUITask();
		ImGuiEx::RenderNotifications();
		m_ImguiOpengl3Layer->EndFrame();
	}

	void VGNodeGraphApp::OnApplicationUpdate(float deltaTime)
	{
		OnFixedUpdate();
		OnUpdate(deltaTime);
		OnRender();
		OnGUI();

		SDL_GL_SwapWindow(m_ApplicationWindow->GetSDLWindow());
	}

	int VGNodeGraphApp::ProcessEvent(const SDL_Event& event)
	{
		m_ApplicationWindow->ProcessEvent(event);

		return 0;
	}

	void VGNodeGraphApp::MakeCurrentRenderContext()
	{
		SDL_GL_MakeCurrent(m_ApplicationWindow->GetSDLWindow(), m_ApplicationWindow->GetContext());
	}
}
