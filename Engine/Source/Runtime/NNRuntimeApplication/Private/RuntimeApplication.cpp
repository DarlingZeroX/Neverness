/**
 * @file RuntimeApplication.cpp
 * @brief SDL3 Runtime Host：Init/Quit、全局事件泵、主窗口帧边界（ImGui）。
 */

#include "RuntimeApplication.h"

#include "Core/SDL3EventTranslator.h"

#include <algorithm>
#include <array>
#include <SDL3/SDL.h>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Core/WindowRegistry.h"
#include "Editor/EditorInitializer.h"
#include "NNFileSystem/Interface/HFileSystem.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "NNRuntimeImGui/Include/imgui/imgui.h"
#include "NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h"
#include "NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h"
#include "NNRuntimeVFS/Include/VFSService.h"
#include <SDL3\SDL_oldnames.h>

namespace NN::Runtime::Application
{
	namespace
	{
		namespace fs = std::filesystem;

		/* 双通道输出：OutputDebugStringA（VS 输出窗口）+ stderr（控制台） */
		void LogError(const char* msg)
		{
#ifdef _WIN32
			OutputDebugStringA("[NNRuntimeApp] ");
			OutputDebugStringA(msg);
			OutputDebugStringA("\n");
#endif
			fprintf(stderr, "[NNRuntimeApp] %s\n", msg);
			fflush(stderr);
		}

		void LogInfo(const char* msg)
		{
#ifdef _WIN32
			OutputDebugStringA("[NNRuntimeApp] ");
			OutputDebugStringA(msg);
			OutputDebugStringA("\n");
#endif
			printf("[NNRuntimeApp] %s\n", msg);
			fflush(stdout);
		}

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

	RuntimeApplication::~RuntimeApplication() = default;

	int RuntimeApplication::Initialize()
	{
		try
		{
			if (m_sdlInitialized)
			{
				LogInfo("Initialize: 已初始化，直接返回 1");
				return 1;
			}

			SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
			LogInfo("Initialize: 开始 SDL_Init");

			if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
			{
				std::string msg = std::string("SDL 初始化失败: ") + SDL_GetError();
				LogError(msg.c_str());
				return -1;
			}
			LogInfo("Initialize: SDL_Init 成功");

			if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false)
			{
				std::string msg = std::string("音频子系统初始化失败: ") + SDL_GetError();
				LogError(msg.c_str());
				return -2;
			}
			LogInfo("Initialize: SDL 音频子系统成功");

			m_sdlInitialized = true;
			m_shouldQuit = false;

			m_eventTranslator = std::make_unique<SDL3EventTranslator>(m_eventQueue);

			std::string editorProjectRootDir;
#ifdef EDITOR_PROJECT_ROOT_DIR
			LogInfo(std::string("Initialize: EDITOR_PROJECT_ROOT_DIR = " + std::string(EDITOR_PROJECT_ROOT_DIR)).c_str());
			editorProjectRootDir = EDITOR_PROJECT_ROOT_DIR;
#endif

			LogInfo("Initialize: 设置 locale");
			try
			{
				std::locale::global(std::locale(".utf8"));
				LogInfo("Initialize: locale 设置成功");
			}
			catch (const std::exception& ex)
			{
				std::string msg = std::string("locale 设置失败: ") + ex.what();
				LogError(msg.c_str());
				return -5;
			}

			LogInfo("Initialize: 查找项目目录");
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

			LogInfo(std::string("Initialize: Project path = " + projectRootDir).c_str());

			if (projectRootDir.empty())
			{
				LogError("Initialize: 未找到可用的编辑器启动项目");
				return -3;
			}

			LogInfo("Initialize: 检查项目目录有效性");
			if (!EditorInitializer::CheckProjectRootDir(projectRootDir))
			{
				std::string msg = std::string("编辑器启动项目无效: ") + projectRootDir;
				LogError(msg.c_str());
				return -4;
			}
			LogInfo("Initialize: 项目目录有效");

			EditorVFSPath paths;
			paths.project = projectRootDir;
			paths.assets = projectRootDir + "/Assets/";
			paths.library = projectRootDir + "/Library/";
			paths.build = projectRootDir + "/Build/";
			paths.packages = projectRootDir + "/Packages/";

			paths.projectSettings = projectRootDir + "/ProjectSettings/";
			paths.projectIntermediate = projectRootDir + "/Intermediate/";
			paths.editor = editorProjectRootDir + "/Resource/Editor/";
			paths.engine = editorProjectRootDir + "/Resource/Engine/";
			EditorInitializer::InitializeVFS(paths);
			LogInfo("Initialize: VFS 初始化完成");

			return 1;
		}
		catch (const std::exception& ex)
		{
			std::string msg = std::string("Initialize 异常: ") + ex.what();
			LogError(msg.c_str());
			return -99;
		}
		catch (...)
		{
			LogError("Initialize 未知异常");
			return -100;
		}
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

			/* 翻译 SDL 事件到 NNEvent 队列（供 C# 消费） */
			if (m_eventTranslator)
			{
				m_eventTranslator->TranslateAndPush(event);
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
		m_eventTranslator.reset();
		m_eventQueue.Clear();

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
