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

#include "Core/Window.h"
#include <SDL3_image/SDL_image.h>
#include "Core/VFS.h"
//#include "Graphics/OpenGL/OpenGL.h"
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>

namespace VisionGal
{
	VGWindow::VGWindow()
	{
		this->AddWindowEventListener([this](const Horizon::Events::HWindowEvent& evt)
			{
				OnWindowEvent.Invoke(evt);
			});
	}

	SDL_HitTestResult VGHitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data)
	{
//#define REPORT_RESIZE_HIT(name) { std::cout << "HIT-TEST: RESIZE_" #name "\n" << std::endl;OnResizeWindowMode = true; return SDL_HITTEST_RESIZE_##name;  }
#define REPORT_RESIZE_HIT(name) { return SDL_HITTEST_RESIZE_##name;  }

		static constexpr int RESIZE_BORDER = 5;
		//bool& OnResizeWindowMode = *reinterpret_cast<bool*>(data);
		VGWindow* window = reinterpret_cast<VGWindow*>(data);

		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		//window->ResizeOnBorderless(pt->x, pt->y);

		if (pt->x < RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOPLEFT);
		}
		else if (pt->x > RESIZE_BORDER && pt->x < w - RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOP);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOPRIGHT);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y > RESIZE_BORDER && pt->y < h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(RIGHT);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOMRIGHT);
		}
		else if (pt->x < w - RESIZE_BORDER && pt->x > RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOM);
		}
		else if (pt->x < RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOMLEFT);
		}
		else if (pt->x < RESIZE_BORDER && pt->y < h - RESIZE_BORDER && pt->y > RESIZE_BORDER) {
			REPORT_RESIZE_HIT(LEFT);
		}

		return SDL_HITTEST_NORMAL;
	}

	void VGWindow::SetInitializeBorderless(bool borderless)
	{
		m_Borderless = borderless;
		//return SDL_SetWindowBordered(GetSDLWindow(), !borderless);
	}

	bool VGWindow::Initialize(const char* window_name, int width, int height, bool allow_resize)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
			return false;

		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
			std::cerr << "音频子系统初始化失败: " << SDL_GetError() << std::endl;
			return false;
		}

		// Submit click events when focusing the window.
		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

		// GL 3.3 Core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		// 创建 Backbuffer
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);    // 设置深度缓冲区大小

		// 设置共享属性（若SDL3支持）  
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

		const float window_size_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
		SDL_PropertiesID props = SDL_CreateProperties();
		SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window_name);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, int(width));
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, int(height));
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, allow_resize);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);

		// 无边框
		//if (m_Borderless)
		//	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);

		// 创建窗口
		SDL_Window* window = SDL_CreateWindowWithProperties(props);
		SDL_DestroyProperties(props);

		// 引擎图标
		std::string iconPath = "/editor/icons/engineIcon.png";
		VFS::SafeReadFileFromVFS(iconPath, [&](const VFS::DataRef& data) {

			const size_t i_ext = iconPath.rfind('.');
			String extension = (i_ext == String::npos ? String() : iconPath.substr(i_ext + 1));

			auto CreateSurface = [&]() { return IMG_LoadTyped_IO(SDL_IOFromMem(data->data(), data->size()), 1, extension.c_str()); };
			SDL_Surface* surface = CreateSurface();

			if (surface) {
				SDL_SetWindowIcon(window, surface);
				SDL_DestroySurface(surface);
			}

			return 0;
			});

		// 设置窗口指针
		SetSDLWindowPtr(window);

		if (!window)
		{
			H_LOG_ERROR("SDL error on create window: %s", SDL_GetError());
			return false;
		}

		m_GLContext = SDL_GL_CreateContext(GetSDLWindow());
		m_OpenGLVersion = "#version 330";

		if (!m_GLContext) {
			SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
			return false;
		}

		// 初始化GLAD
		//if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		//	H_LOG_ERROR("Failed to initialize GLAD");
		//	return 1;
		//}
		const int gl_version = gladLoaderLoadGL();
		if (gl_version == 0)
		{
			H_LOG_ERROR("Failed to initialize OpenGL context.");
			return false;
		}

		// 初始化鼠标和键盘
		//InitialMouseKeyboard();

		//注意：还必须在按下字符键时启用 SDL 的文本输入模式：
		//SDL_StartTextInput(window);   // 在应用开始时调用

		// 设置无边框可调整大小
		if (m_Borderless)
		{
			SDL_SetWindowBordered(window, !m_Borderless);
			SDL_GetWindowID(GetSDLWindow());
			SDL_SetWindowResizable(GetSDLWindow(), true);
			SDL_SetWindowHitTest(GetSDLWindow(), VGHitTestCallback, this);

			//HideTilteBar::HideTitleBar(window);
		}

		// 设置窗口获得焦点
		SDL_RaiseWindow(GetSDLWindow());

		return true;
	}
}
