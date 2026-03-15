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

#include "Engine/VGEngine.h"
#include "Resource/ResourceManager.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGCore/Include/Core/VFS.h"
#include "Engine/Manager.h"
#include "Scene/GameActorFactory.h"
#include <HCorePlatform/Include/NativeFileDialog/portable-file-dialogs.h>

#include "HCore/Interface/HStringTools.h"

#include <exception>
#include <iostream>
#include <sstream>

namespace VisionGal
{
	namespace
	{
		// 辅助：报告异常并弹窗
		inline void ReportException(const std::exception* e, const char* context)
		{
			std::string msg;
			if (e)
			{
				msg = Horizon::HStringTools::Format("Unhandled exception in %s: %s", context, e->what());
			}
			else
			{
				msg = Horizon::HStringTools::Format("Unknown exception in %s", context);
			}

			// 弹出错误对话框
			try
			{
				pfd::message("Error", msg, pfd::choice::ok, pfd::icon::error);
			}
			catch (...)
			{
				// 如果弹窗失败，也要保证至少在 stderr 输出
			}

			// 输出到标准错误
			std::cerr << msg << std::endl;
		}
	}

	VGEngine::VGEngine()
	{
		m_LastUpdateTime = std::chrono::high_resolution_clock::now();

		// 断言错误记录
		//_CrtSetReportHook([](int reportType, char* message, int* retVal)
		//{
		//	H_LOG_ERROR(message);
		//	return 0;
		//});
	}

	VGEngine* VGEngine::Get()
	{
		static VGEngine* s_VGEngine = new VGEngine();
		return s_VGEngine;
	}

	bool VGEngine::LoadProject()
	{
		Initialize();

		// 加载项目配置
		ProjectSettings::LoadProjectSettings();

		CreateResourceManagers();
		return true;
	}

	ProjectSettings& VGEngine::GetProjectConfig()
	{
		return m_ProjectConfig;
	}

	void VGEngine::LoadEditorMainScene()
	{
		auto& setting = ProjectSettings::GetProjectSettings().Editor;

		if (setting.MainScene.empty())
		{
			H_LOG_INFO("Editor default scene no exist!");
			m_Scene = CreateRef<Scene>();
			GetSceneManager()->SetCurrentScene(m_Scene);
			return;
		}

		m_Scene = GetSceneManager()->LoadScene(setting.MainScene);

		if (m_Scene == nullptr)
		{
			m_Scene = CreateRef<Scene>();
			GetSceneManager()->SetCurrentScene(m_Scene);
		}
	}

	void VGEngine::Initialize()
	{
        Core::Initialize();
	}

	void VGEngine::OnUpdateSubSystem(float deltaTime)
	{
		try
		{
			GetViewportManager()->FrameUpdate();
		}
		catch (const std::exception& e)
		{
			ReportException(&e, "OnUpdateSubSystem");
			// 视情况决定是否结束程序，这里仅记录并继续
		}
		catch (...)
		{
			ReportException(nullptr, "OnUpdateSubSystem");
		}
	}

	void VGEngine::LoadProjectMainScene()
	{
		auto& setting = ProjectSettings::GetProjectSettings().Application;
		m_Scene = GetSceneManager()->LoadScene(setting.RunningMainScene);

		if (m_Scene == nullptr)
		{
			std::string error = Horizon::HStringTools::Format("Invalid project main scene %s", setting.RunningMainScene.c_str());
			pfd::message("Error", error, pfd::choice::ok, pfd::icon::error);

			RequestExit();
		}
	}

	void VGEngine::Run() {
		using clock = std::chrono::high_resolution_clock;
		const double FIXED_TIME = 1.0 / 60.0;
		const double MAX_FRAME_TIME = 0.25; // 防止长帧

		double updateLag = 0.0;
		auto lastTime = clock::now();

		while (ProcessEvents()) {
			try
			{
				// 1. 计算时间间隔 (deltaTime)
				auto now = clock::now();
				double deltaTime = std::chrono::duration<double>(now - lastTime).count();
				deltaTime = std::min(deltaTime, MAX_FRAME_TIME);
				lastTime = now;

				updateLag += deltaTime;

				// 2. 固定更新
				while (updateLag >= FIXED_TIME) {
					//FixedUpdate(FIXED_TIME);
					updateLag -= FIXED_TIME;
				}

				// 3. 渲染和应用更新
				//float alpha = static_cast<float>(updateLag / FIXED_TIME);
				OnUpdateSubSystem(static_cast<float>(deltaTime));
				OnApplicationUpdate(static_cast<float>(deltaTime));
				//RenderWithInterpolation(alpha);

				// 4. 可选帧率控制
				double frameTime = std::chrono::duration<double>(clock::now() - now).count();
				if (frameTime < FIXED_TIME) {
					std::this_thread::sleep_for(std::chrono::duration<double>(FIXED_TIME - frameTime));
				}
			}
			catch (const std::exception& e)
			{
				ReportException(&e, "Run");
				RequestExit();
			}
			catch (...)
			{
				ReportException(nullptr, "Run");
				RequestExit();
			}
		}
	}

	void VGEngine::AddApplication(Ref<IEngineApplication> layer)
	{
		m_Applications.push_back(layer);
	}

	void VGEngine::OnApplicationUpdate(float deltaTime)
	{
        for (auto& app : m_Applications)
        {
			try
			{
				app->MakeCurrentRenderContext();
				app->OnApplicationUpdate(deltaTime);
			}
			catch (const std::exception& e)
			{
				ReportException(&e, "OnApplicationUpdate - application");
				// 单个应用异常，不影响其他应用
			}
			catch (...)
			{
				ReportException(nullptr, "OnApplicationUpdate - application");
			}
        }
	}

    bool VGEngine::ProcessEvents()
	{
#if defined RMLUI_PLATFORM_EMSCRIPTEN

		// Ideally we would hand over control of the main loop to emscripten:
		//
		//  // Hand over control of the main loop to the WebAssembly runtime.
		//  emscripten_set_main_loop_arg(EventLoopIteration, (void*)user_data_handle, 0, true);
		//
		// The above is the recommended approach. However, as we don't control the main loop here we have to make due with another approach. Instead, use
		// Asyncify to yield by sleeping.
		// Important: Must be linked with option -sASYNCIFY
		// See https://emscripten.org/docs/porting/asyncify.html for more information.

		// 理想情况下，我们应该将主循环的控制权交给 Emscripten：
		//
		//   // 将主循环的控制权交给 WebAssembly 运行时
		//   emscripten_set_main_loop_arg(EventLoopIteration, (void*)user_data_handle, 0, true);
		//
		// 上面是推荐的方式。然而，由于我们在这里无法控制主循环，因此只能采用另一种方式。
		// 我们将使用 Asyncify，通过休眠的方式让出控制权。
		// 重要提示：链接时必须使用选项 -sASYNCIFY
		// 更多信息请参考：https://emscripten.org/docs/porting/asyncify.html

		emscripten_sleep(1);

#endif
		bool has_event = false;
		bool result = m_Running;
		m_Running = true;

		try
		{
			SDL_PumpEvents();

			SDL_Event ev;
			//if (m_PowerSave)
			//	has_event = SDL_WaitEventTimeout(&ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0) * 1000));
			//else
			//	has_event = SDL_PollEvent(&ev);
			has_event = SDL_PollEvent(&ev);

			while (has_event)
			{
				switch (ev.type)
				{
				case SDL_EVENT_QUIT:
					return false;
				//case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				//	return false;
				}

				// 交给窗口处理事件
				for (auto& app : m_Applications)
				{
					app->MakeCurrentRenderContext();
					app->ProcessEvent(ev);
				}

				has_event = SDL_PollEvent(&ev);
			}
		}
		catch (const std::exception& e)
		{
			ReportException(&e, "ProcessEvents");
			// 出现异常时尽量安全退出
			RequestExit();
			return false;
		}
		catch (...)
		{
			ReportException(nullptr, "ProcessEvents");
			RequestExit();
			return false;
		}

		return result;
	}

    void VGEngine::Shutdown()
    {
		SDL_Quit();
    }

    void VGEngine::RequestExit()
    {
        m_Running = false;
    }

}
