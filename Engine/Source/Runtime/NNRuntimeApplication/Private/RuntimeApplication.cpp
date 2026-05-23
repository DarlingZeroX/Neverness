/**
 * @file RuntimeApplication.cpp
 * @brief SDL3 Runtime Host：Init/Quit、全局事件泵、主窗口帧边界（ImGui）。
 */

#include "RuntimeApplication.h"

#include <algorithm>
#include <array>
#include <SDL3/SDL.h>
#include <cctype>
#include <filesystem>
#include <iostream>

#include "Core/WindowRegistry.h"
#include "Editor/EditorInitializer.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "NNRuntimeImGui/Include/imgui/imgui.h"
#include "NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h"
#include "NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h"
#include "NNRuntimeVFS/Include/VFSService.h"

namespace NN::Runtime::Application
{
	namespace
	{
		namespace fs = std::filesystem;

		std::string TrimWhitespace(std::string value)
		{
			auto notSpace = [](unsigned char ch) {
				return !std::isspace(ch);
			};

			value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
			value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
			return value;
		}

		bool LooksLikeProjectRoot(const fs::path& path)
		{
			std::error_code ec;
			if (!fs::is_directory(path, ec))
			{
				return false;
			}

			return fs::is_directory(path / "Assets", ec) &&
				fs::is_directory(path / "ProjectSettings", ec) &&
				fs::is_directory(path / "Intermediate", ec);
		}

		std::string FindDefaultProjectRoot(const std::string& editorProjectRootDir)
		{
			if (editorProjectRootDir.empty())
			{
				return {};
			}

			const fs::path editorRoot(editorProjectRootDir);
			const std::array<fs::path, 3> preferredCandidates = {
				editorRoot / "Project" / "示例项目",
				editorRoot / "Project" / "Test Project",
				editorRoot / "Projects" / "Test Project",
			};

			for (const auto& candidate : preferredCandidates)
			{
				if (LooksLikeProjectRoot(candidate))
				{
					return candidate.string();
				}
			}

			const fs::path projectRoot = editorRoot / "Project";
			std::error_code ec;
			if (!fs::is_directory(projectRoot, ec))
			{
				return {};
			}

			for (const auto& entry : fs::directory_iterator(projectRoot, ec))
			{
				if (ec)
				{
					break;
				}

				if (entry.is_directory(ec) && LooksLikeProjectRoot(entry.path()))
				{
					return entry.path().string();
				}
			}

			return {};
		}
	} // namespace

	bool RuntimeApplication::Initialize()
	{
		if (m_sdlInitialized)
		{
			return true;
		}

		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
		{
			std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
			return false;
		}

		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false)
		{
			std::cerr << "音频子系统初始化失败: " << SDL_GetError() << std::endl;
			return false;
		}

		m_sdlInitialized = true;
		m_shouldQuit = false;

		std::string editorProjectRootDir;
#ifdef EDITOR_PROJECT_ROOT_DIR
		std::cout << "VisionGal Project root is: " << EDITOR_PROJECT_ROOT_DIR << std::endl;
		editorProjectRootDir = EDITOR_PROJECT_ROOT_DIR;
#endif

		std::locale::global(std::locale(".utf8"));

		std::string projectRootDir = FindDefaultProjectRoot(editorProjectRootDir);
		std::string projectPath;
		if (NN::Core::HFileSystem::ReadTextFromFile("Data/EditorStartupData.txt", projectPath))
		{
			projectPath = TrimWhitespace(projectPath);
			if (!projectPath.empty())
			{
				projectRootDir = projectPath;
			}
		}

		H_LOG_INFO("Project path: %s", projectRootDir.c_str());

		if (projectRootDir.empty())
		{
			std::cerr
				<< "未找到可用的编辑器启动项目。请创建 Data/EditorStartupData.txt 指向项目目录，"
				<< "或确保仓库下存在有效的 Project/* 项目目录。"
				<< std::endl;
			return false;
		}

		if (!EditorInitializer::CheckProjectRootDir(projectRootDir))
		{
			std::cerr << "编辑器启动项目无效: " << projectRootDir << std::endl;
			return false;
		}

		EditorVFSPath paths;
		paths.assets = projectRootDir + "/Assets/";
		paths.projectSettings = projectRootDir + "/ProjectSettings/";
		paths.projectIntermediate = projectRootDir + "/Intermediate/";
		paths.editor = editorProjectRootDir + "/Resource/Editor/";
		paths.engine = editorProjectRootDir + "/Resource/Engine/";
		EditorInitializer::InitializeVFS(paths);

		return true;
	}

	void RuntimeApplication::OnPrimaryWindowCreated(NNWindowHandle handle)
	{
		if (handle == NN_INVALID_WINDOW_HANDLE || m_imguiAttached)
		{
			return;
		}

		m_primaryWindowHandle = handle;
		if (VGWindow* window = WindowRegistry::Resolve(handle))
		{
			AddImguiLayer(window);
			m_imguiAttached = true;
		}
	}

	bool RuntimeApplication::PumpEvents()
	{
		if (!m_sdlInitialized)
		{
			return false;
		}

		if (m_shouldQuit)
		{
			return false;
		}

		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				m_shouldQuit = true;
				return false;
			}

			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				m_shouldQuit = true;
				return false;
			}

			if (VGWindow* primary = WindowRegistry::Resolve(m_primaryWindowHandle))
			{
				if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
				{
					//SDL_Window* evtWindow = SDL_GetWindowFromID(event.window.windowID);
					//if (evtWindow == primary->GetSDLWindow())
					// 目前是Window自己判断是否处理窗口事件，因为有ImGui的窗口事件分发机制，如果在RuntimeApplication这里过滤窗口事件，可能会导致ImGui的窗口事件无法正确分发到对应窗口。
					{
						primary->ProcessEvent(event);
					}
				}
				else
				{
					primary->ProcessEvent(event);
				}
			}
		}

		return !m_shouldQuit;
	}

	void RuntimeApplication::Shutdown()
	{
		m_ImguiOpengl3Layer.reset();
		m_imguiAttached = false;
		m_primaryWindowHandle = NN_INVALID_WINDOW_HANDLE;

		WindowRegistry::DestroyAll();

		if (m_sdlInitialized)
		{
			SDL_Quit();
			m_sdlInitialized = false;
		}

		m_shouldQuit = false;
	}

	void RuntimeApplication::BeginFrame()
	{
		if (m_ImguiOpengl3Layer)
		{
			m_ImguiOpengl3Layer->BeginFrame();
		}
	}

	void RuntimeApplication::EndFrame()
	{
		if (m_ImguiOpengl3Layer)
		{
			m_ImguiOpengl3Layer->EndFrame();
		}

		if (VGWindow* primary = WindowRegistry::Resolve(m_primaryWindowHandle))
		{
			SDL_GL_SwapWindow(primary->GetSDLWindow());
		}
	}

	void RuntimeApplication::AddImguiLayer(VGWindow* window)
	{
		if (window == nullptr)
		{
			return;
		}

		window->AddLayer(std::make_unique<ImGuiEx::Opengl3ImGuiWindowLayer>(window));
		m_ImguiOpengl3Layer = std::make_unique<NN::Runtime::ImguiOpengl3Layer>(window, window->GetContext());

		ImGuiIO& io = ImGui::GetIO();

		{
			NN::Runtime::VFS::VFSService::SafeReadFileFromVFS(NN::Runtime::RuntimeCore::GetEngineResourcePathVFS() + "fonts/msyh.ttc", [&](const NN::Runtime::VFS::VFSService::DataRef& data) {
				ImFontConfig icons_config;
				icons_config.FontDataOwnedByAtlas = false;

				io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 17, &icons_config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());

				return 0;
				});
		}

		{
			NN::Runtime::VFS::VFSService::SafeReadFileFromVFS(NN::Runtime::RuntimeCore::GetEngineResourcePathVFS() + "fonts/fa-regular-400.ttf", [&](const NN::Runtime::VFS::VFSService::DataRef& data) {
				static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

				ImFontConfig icons_config;
				icons_config.MergeMode = true;
				icons_config.PixelSnapH = true;
				icons_config.GlyphOffset = ImVec2(0, 2);
				icons_config.FontDataOwnedByAtlas = false;

				io.Fonts->AddFontFromMemoryTTF(data->data(), data->size(), 17, &icons_config, icons_ranges);
				return 0;
				});
		}

		io.Fonts->Build();

		H_LOG_INFO("Initializing ImGui fonts");
	}
} // namespace NN::Runtime::Application
