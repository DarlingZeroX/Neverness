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
#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h>
#include <NNRuntimeImGui/Include/Imgui/imgui_impl_opengl3.h>
#include <NNEditorFramework/Framework.h>
#include "NNRuntimeCore/Include/Core/VFS.h"
#include "NNEditorFramework/Interface/AssetEditor.h"
#if defined(VISIONGAL_BUILD_LEGACY_GALGAME)
#include "VGGalgame/Interface/GalgameSystem.h"
#include "VGEditorGalgame/Interface/ModuleEditorGalgame.h"
#endif
#include "NNEditorFramework/Interface/ModuleEditorFramework.h"

namespace VisionGal::Editor
{
	VGEditorApplication::~VGEditorApplication()
	{
	}
	 
	void VGEditorApplication::Initialize()
	{
		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		auto editorName = editorConfig.EditorName;
		auto windowWidth = editorConfig.EditorWindowWidth;
		auto windowHeight = editorConfig.EditorWindowHeight;

		// 初始化编辑器窗口
		m_EditorWindow = NN::MakeRef<NN::Runtime::VGWindow>();
		m_EditorWindow->SetInitializeBorderless(true);
		m_EditorWindow->Initialize(editorName.c_str(), windowWidth, windowHeight,true);

		// 初始化游戏引擎
		m_GameEngine = NN::MakeRef<NN::Runtime::CoreGameEngine>();
		m_GameEngine->Initialize(m_EditorWindow.get());

#if defined(VISIONGAL_BUILD_LEGACY_GALGAME)
		// 初始化GalGame子系统
		GalGameSystem::Initialize(*m_GameEngine);
#endif

		// 初始化ImGui
		AddImguiLayer();
		InitializeEditorUI();

		// 挂载编辑器模块
		NN::Editor::ModuleEditorFramework::MountToEditor(m_EditorWindow, m_GameEngine);
#if defined(VISIONGAL_BUILD_LEGACY_GALGAME)
		ModuleEditorGalGame::MountToEditor();
		AssetEditor::Get().OpenAsset("/assets/GalGameVisualScript0.vgasset");
#endif
	}

	void VGEditorApplication::AddApplicationLayer(NN::Runtime::IEngineApplicationLayer* layer)
	{
	}

	void VGEditorApplication::InitializeEditorUI()
	{
		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		// 设置本地语言
		NN::Editor::EditorLoadLanguage(editorConfig.EditorLanguage);
		// 设置主题
		NN::Editor::EditorStyle::SetTheme(editorConfig.EditorTheme);
	}

	void VGEditorApplication::AddImguiLayer()
	{
		// 添加ImGui Layer
		m_EditorWindow->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(m_EditorWindow.get()));
		m_ImguiOpengl3Layer = std::make_unique<NN::Runtime::ImguiOpengl3Layer>(m_EditorWindow.get(), m_EditorWindow->GetContext());

		ImGuiIO& io = ImGui::GetIO();

		auto& editorConfig = NN::Editor::EditorCore::GetEditorPreferences().Editor;

		//if (m_EditorConfig.LoadFontChinese)
		{
			NN::Runtime::VFS::SafeReadFileFromVFS(NN::Runtime::Core::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const NN::Runtime::VFS::DataRef& data) {
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
			NN::Runtime::VFS::SafeReadFileFromVFS(NN::Runtime::Core::GetEngineResourcePathVFS() + "fonts/fa-regular-400.ttf", [&](const NN::Runtime::VFS::DataRef& data) {
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
		NN::Editor::PanelManager::GetInstance()->OnUpdate(deltaTime);
	}

	void VGEditorApplication::OnFixedUpdate()
	{
		NN::Editor::PanelManager::GetInstance()->OnFixedUpdate();
	}

	void VGEditorApplication::OnGUI()
	{
		//m_ImguiOpengl3Layer->BeginFrame();
		NN::Editor::PanelManager::GetInstance()->OnGUI();
		ImGuiEx::ImTaskManager::Get().RenderUITask();
		ImGuiEx::RenderNotifications();
		//m_ImguiOpengl3Layer->EndFrame();
	}

	void VGEditorApplication::OnApplicationUpdate(float deltaTime)
	{
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
