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

#include "EngineEditor.h"
#include <HFileSystem/Interface/HFileSystem.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGImgui/Include/ImGuiLayer/SDL3Decorator.h>
#include <VGImgui/Include/Imgui/imgui_impl_opengl3.h>
#include <VGEditorComponent/Framework.h>
//#include <VGEngine/Include/UI/UISystem.h>
#include <VGEditorCore/IncludeCore.h>
#include "VGCore/Include/Core/VFS.h"
#include "VGEditorComGalgame/Interface/VGEditorGalgame.h"
#include "VGGalgame/Interface/GalgameSystem.h"

namespace VisionGal::Editor
{
	VGEditorApplication::~VGEditorApplication()
	{
	}
	 
	void VGEditorApplication::Initialize()
	{
		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		auto editorName = editorConfig.EditorName;
		auto windowWidth = editorConfig.EditorWindowWidth;
		auto windowHeight = editorConfig.EditorWindowHeight;

		// 初始化编辑器窗口
		m_EditorWindow = MakeRef<VGWindow>();
		m_EditorWindow->SetInitializeBorderless(true);
		m_EditorWindow->Initialize(editorName.c_str(), windowWidth, windowHeight,true);

		// 初始化游戏引擎
		m_GameEngine = MakeRef<CoreGameEngine>();
		m_GameEngine->Initialize(m_EditorWindow.get());

		// 初始化GalGame子系统
		GalGameSystem::Initialize(*m_GameEngine);

		// 初始化ImGui
		AddImguiLayer();
		InitializeEditorUI();

		// 初始化内容浏览器
		String contentPath = VFS::GetInstance()->AbsolutePath(Core::GetAssetsPathVFS());
		ContentBrowser::Create(contentPath);

		// 初始化资源导入管理器
		AssetImporterManager::GetInstance().Initialize(m_EditorWindow);

		// 初始化GalGame编辑器组件
		VGEditorGalGame::Initialize();
	}

	void VGEditorApplication::AddApplicationLayer(IEngineApplicationLayer* layer)
	{
	}

	void VGEditorApplication::InitializeEditorPanels()
	{
		auto* editor = PanelManager::GetInstance();

		auto mainWindow = MakeRef<EditorMainWindow>();
		//auto editorSidebar = MakeRef<EditorSideBar>();

		editor->AddPanelWithID("EditorMenuBar", MakeRef<EditorMenuBar>(m_EditorWindow.get()));
		//editor->AddPanelWithID("EditorSideBar", editorSidebar);
		editor->AddPanelWithID("EditorMainWindow", mainWindow);
		editor->AddPanelWithID("EditorPreferences", MakeRef<PreferencesPanel>());
		editor->AddPanelWithID("ProjectSetting", MakeRef<ProjectSettingPanel>());
		editor->AddPanelWithID("BuildSettings", MakeRef<BuildSettingsPanel>());

		mainWindow->AddPanelWithID("ContentBrowserPanel", MakeRef<ContentBrowserPanel>());
		mainWindow->AddPanelWithID("ConsolePanel", MakeRef<ConsolePanel>());
		mainWindow->AddPanelWithID("SceneBrowserPanel", MakeRef<SceneBrowserPanel>());
		mainWindow->AddPanelWithID("DetailBrowserPanel", MakeRef<DetailBrowserPanel>());
		mainWindow->AddPanelWithID("EditorViewport", MakeRef<EditorViewport>(m_GameEngine->GetViewport()));
		mainWindow->AddPanelWithID("CodeStudioPanel", MakeRef<CodeStudioPanel>());
	}

	void VGEditorApplication::InitializeEditorUI()
	{
		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		// 设置本地语言
		EditorLoadLanguage(editorConfig.EditorLanguage);

		// 设置主题
		EditorStyle::SetTheme(editorConfig.EditorTheme);

		// 创建Imgui的UI任务执行器
		ImGuiEx::ImTaskManager::CreateManager();
	}

	void VGEditorApplication::AddImguiLayer()
	{
		// 添加ImGui Layer
		m_EditorWindow->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(m_EditorWindow.get()));
		m_ImguiOpengl3Layer = std::make_unique<ImguiOpengl3Layer>(m_EditorWindow.get(), m_EditorWindow->GetContext());

		ImGuiIO& io = ImGui::GetIO();

		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		//if (m_EditorConfig.LoadFontChinese)
		{
			VFS::SafeReadFileFromVFS(Core::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const VFS::DataRef& data) {
				ImGuiIO& io = ImGui::GetIO();
				ImFontConfig icons_config;
				icons_config.FontDataOwnedByAtlas = false;

				//io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 17, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
				io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), editorConfig.EditorFontSize, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

				return 0;
				});
		}
		//else
		//{
		//	ImGui::GetIO().Fonts->AddFontDefault();
		//}

		//if (m_EditorConfig.LoadFontAwesome)
		{
			VFS::SafeReadFileFromVFS(Core::GetEngineResourcePathVFS() + "fonts/fa-regular-400.ttf", [&](const VFS::DataRef& data) {
				ImGuiIO& io = ImGui::GetIO();
				static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

				ImFontConfig icons_config;
				icons_config.MergeMode = true;
				icons_config.PixelSnapH = true;
				icons_config.GlyphOffset = ImVec2(0, 2);
				icons_config.FontDataOwnedByAtlas = false;

				io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), editorConfig.EditorFontSize, &icons_config, icons_ranges);
				return 0;
				});
		}

		io.Fonts->Build();
	}

	void VGEditorApplication::OnRender()
	{
		m_GameEngine->OnRender();
	}

	void VGEditorApplication::OnUpdate(float deltaTime)
	{
		m_GameEngine->OnUpdate(deltaTime);
		PanelManager::GetInstance()->OnUpdate(deltaTime);

		//auto size =m_EditorWindow->GetWindowSize();
		//std::cout << "W: " << size.x << " H: " << size.y << std::endl;
	}

	void VGEditorApplication::OnFixedUpdate()
	{
		PanelManager::GetInstance()->OnFixedUpdate();
	}

	void VGEditorApplication::OnGUI()
	{
		//m_ImguiOpengl3Layer->BeginFrame();
		PanelManager::GetInstance()->OnGUI();
		ImGuiEx::ImTaskManager::GetInstance().RenderUITask();
		ImGuiEx::RenderNotifications();
		//m_ImguiOpengl3Layer->EndFrame();
	}

	void VGEditorApplication::OnApplicationUpdate(float deltaTime)
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		OnFixedUpdate();
		OnUpdate(deltaTime);
		m_ImguiOpengl3Layer->BeginFrame();
		OnRender();
		OnGUI();
		m_ImguiOpengl3Layer->EndFrame();

		SDL_GL_SwapWindow(m_EditorWindow->GetSDLWindow());
	}

	int VGEditorApplication::ProcessEvent(const SDL_Event& event)
	{
		return m_EditorWindow->ProcessEvent(event);
	}

	void VGEditorApplication::MakeCurrentRenderContext()
	{
		SDL_GL_MakeCurrent(m_EditorWindow->GetSDLWindow(), m_EditorWindow->GetContext());
	}
}
