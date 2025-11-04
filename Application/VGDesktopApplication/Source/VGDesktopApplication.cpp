#include "VGDesktopApplication.h"
#include <HCore/Include/System/HFileSystem.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGImgui/Include/ImGuiLayer/SDL3Decorator.h>
#include <VGImgui/Include/Imgui/imgui_impl_opengl3.h>
#include <VGEditorFramework/Framework.h>
#include <VGEngine/Include/UI/UISystem.h>
#include "VGEditorFramework/Include/EditorCore/EditorCore.h"
#include "VGEngine/Include/Core/VFS.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGEngine/Include/Project/ProjectSettings.h"

namespace VisionGal::Editor
{
	VGDesktopApplication::~VGDesktopApplication()
	{
	}
	 
	void VGDesktopApplication::Initialize()
	{
		auto& editorConfig = ProjectSettings::GetProjectSettings();

		auto editorName = editorConfig.Application.ApplicationName;
		auto windowWidth = editorConfig.Application.WindowWidth;
		auto windowHeight = editorConfig.Application.WindowHeight;

		m_ApplicationWindow = CreateRef<VGWindow>();
		m_ApplicationWindow->Initialize(editorName.c_str(), windowWidth, windowHeight,true);

		m_GameEngine = CreateRef<CoreGameEngine>();
		m_GameEngine->Initialize(m_ApplicationWindow.get());

		AddImguiLayer();
		InitializeEditorUI();
	}

	void VGDesktopApplication::AddApplicationLayer(IEngineApplicationLayer* layer)
	{
	}

	void VGDesktopApplication::InitializeEditorPanels()
	{
		auto* editor = PanelManager::GetInstance();

		//editor->AddPanelWithID("EditorMenuBar", CreateRef<EditorMenuBar>());
		//editor->AddPanelWithID("EditorPreferences", CreateRef<PreferencesPanel>());
		//editor->AddPanelWithID("ProjectSetting", CreateRef<ProjectSettingPanel>());

		//editor->AddPanelWithID("EditorViewport", CreateRef<EditorViewport>(m_GameEngine->GetViewport()));
	}

	void VGDesktopApplication::RunScene()
	{
		// 进入播放模式
		GetSceneManager()->EnterPlayMode();
		// 设置渲染到屏幕
		m_GameEngine->SetRenderFinalResultToScreen(true);

		// 初始化视口
		auto* viewport = m_GameEngine->GetViewport();
		viewport->SetWindowID(SDL_GetWindowID(m_ApplicationWindow->GetSDLWindow()));
		viewport->SetViewportSize({ 10, 10 });
	}

	void VGDesktopApplication::InitializeEditorUI()
	{
		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		// 设置本地语言
		//EditorLoadLanguage(editorConfig.EditorLanguage);
		//
		//// 设置主题
		//EditorStyle::SetTheme(editorConfig.EditorTheme);
		EditorStyle::DarkTheme();

		// 创建Imgui的UI任务执行器
		ImGuiEx::ImTaskManager::CreateManager();
	}

	void VGDesktopApplication::AddImguiLayer()
	{
		// 添加ImGui Layer
		m_ApplicationWindow->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(m_ApplicationWindow.get()));
		m_ImguiOpengl3Layer = std::make_unique<ImguiOpengl3Layer>(m_ApplicationWindow.get(), m_ApplicationWindow->GetContext());

		ImGuiIO& io = ImGui::GetIO();

		auto& editorConfig = EditorCore::GetEditorPreferences().Editor;

		VFS::SafeReadFileFromVFS(Core::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const VFS::DataRef& data) {
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig icons_config;
			icons_config.FontDataOwnedByAtlas = false;

			//io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 17, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
			io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), editorConfig.EditorFontSize, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

			return 0;
			});
	}

	void VGDesktopApplication::OnRender()
	{
		m_GameEngine->OnRender();
	}

	void VGDesktopApplication::OnUpdate(float deltaTime)
	{
		// 桌面应用程序需要实时更新视口
		auto* viewport = m_GameEngine->GetViewport();
		viewport->SetWindowID(SDL_GetWindowID(m_ApplicationWindow->GetSDLWindow()));
		viewport->SetViewportPosition({ 0, 0 });
		viewport->SetViewportSize({ m_ApplicationWindow->WindowWidth(), m_ApplicationWindow->WindowHeight() });

		// 更新游戏引擎
		m_GameEngine->OnUpdate(deltaTime);
		PanelManager::GetInstance()->OnUpdate(deltaTime);
	}

	void VGDesktopApplication::OnFixedUpdate()
	{
		PanelManager::GetInstance()->OnFixedUpdate();
	}

	void VGDesktopApplication::OnGUI()
	{
		m_ImguiOpengl3Layer->BeginFrame();
		PanelManager::GetInstance()->OnGUI();
		ImGuiEx::ImTaskManager::GetInstance().RenderUITask();
		ImGuiEx::RenderNotifications();
		m_ImguiOpengl3Layer->EndFrame();
	}

	void VGDesktopApplication::OnApplicationUpdate(float deltaTime)
	{
		OnFixedUpdate();
		OnUpdate(deltaTime);
		OnRender();
		OnGUI();

		SDL_GL_SwapWindow(m_ApplicationWindow->GetSDLWindow());
	}

	int VGDesktopApplication::ProcessEvent(const SDL_Event& event)
	{
		m_ApplicationWindow->ProcessEvent(event);

		return 0;
	}

	void VGDesktopApplication::MakeCurrentRenderContext()
	{
		SDL_GL_MakeCurrent(m_ApplicationWindow->GetSDLWindow(), m_ApplicationWindow->GetContext());
	}
}
