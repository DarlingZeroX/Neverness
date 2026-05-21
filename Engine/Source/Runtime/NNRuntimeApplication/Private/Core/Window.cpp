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
#include <NNCore/Interface/HLog.h>
#include "NNRuntimeVFS/Include/VFSService.h"
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>

namespace NN::Runtime
{
	VGWindow::VGWindow()
	{
		this->AddWindowEventListener([this](const NN::Core::Events::HWindowEvent& evt)
			{
				OnWindowEvent.Invoke(evt);
			});
	}

	SDL_HitTestResult VGHitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data)
	{
#define REPORT_RESIZE_HIT(name) { return SDL_HITTEST_RESIZE_##name;  }

		static constexpr int RESIZE_BORDER = 5;
		VGWindow* window = reinterpret_cast<VGWindow*>(data);
		(void)window;

		int w, h;
		SDL_GetWindowSize(win, &w, &h);

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
	}

	bool VGWindow::CreateFromDesc(const NNWindowDesc* desc)
	{
		const char* windowName = "Neverness";
		int width = 1280;
		int height = 720;
		bool allowResize = true;
		bool maximized = false;
		bool hidden = false;

		if (desc != nullptr)
		{
			if (desc->title != nullptr && desc->title[0] != '\0')
			{
				windowName = desc->title;
			}
			if (desc->width > 0)
			{
				width = desc->width;
			}
			if (desc->height > 0)
			{
				height = desc->height;
			}
			allowResize = desc->resizable;
			maximized = desc->maximized;
			hidden = desc->hidden;
		}

		return Initialize(windowName, width, height, allowResize)
			&& ([this, maximized, hidden]() {
				SDL_Window* window = GetSDLWindow();
				if (window == nullptr)
				{
					return false;
				}
				if (maximized)
				{
					SDL_MaximizeWindow(window);
				}
				if (hidden)
				{
					SDL_HideWindow(window);
				}
				else
				{
					SDL_ShowWindow(window);
				}
				return true;
			}());
	}

	bool VGWindow::Initialize(const char* window_name, int width, int height, bool allow_resize)
	{
		// 注意：SDL_Init 由 RuntimeApplication::Initialize 统一调用，此处不再重复初始化。

		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

		SDL_PropertiesID props = SDL_CreateProperties();
		SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window_name);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, int(width));
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, int(height));
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, allow_resize);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);

		SDL_Window* window = SDL_CreateWindowWithProperties(props);
		SDL_DestroyProperties(props);

		std::string iconPath = "/editor/icons/engineIcon.png";
		VFS::VFSService::SafeReadFileFromVFS(iconPath, [&](const VFS::VFSService::DataRef& data) {

			const size_t i_ext = iconPath.rfind('.');
			std::string extension = (i_ext == std::string::npos ? std::string() : iconPath.substr(i_ext + 1));

			auto CreateSurface = [&]() { return IMG_LoadTyped_IO(SDL_IOFromMem(data->data(), data->size()), 1, extension.c_str()); };
			SDL_Surface* surface = CreateSurface();

			if (surface) {
				SDL_SetWindowIcon(window, surface);
				SDL_DestroySurface(surface);
			}

			return 0;
			});

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

		const int gl_version = gladLoaderLoadGL();
		if (gl_version == 0)
		{
			H_LOG_ERROR("Failed to initialize OpenGL context.");
			return false;
		}

		if (m_Borderless)
		{
			SDL_SetWindowBordered(window, !m_Borderless);
			SDL_GetWindowID(GetSDLWindow());
			SDL_SetWindowResizable(GetSDLWindow(), true);
			SDL_SetWindowHitTest(GetSDLWindow(), VGHitTestCallback, this);
		}

		SDL_RaiseWindow(GetSDLWindow());

		return true;
	}

	void VGWindow::SetWindowTitle(const char* title)
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_SetWindowTitle(w, title != nullptr ? title : "Neverness");
		}
	}

	void VGWindow::SetWindowPixelSize(int width, int height)
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_SetWindowSize(w, width, height);
		}
	}

	void VGWindow::GetWindowPixelSize(int* outWidth, int* outHeight) const
	{
		if (outWidth == nullptr || outHeight == nullptr)
		{
			return;
		}
		*outWidth = 0;
		*outHeight = 0;
		if (SDL_Window* w = const_cast<VGWindow*>(this)->GetSDLWindow())
		{
			SDL_GetWindowSize(w, outWidth, outHeight);
		}
	}

	void VGWindow::SetWindowScreenPosition(int x, int y)
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_SetWindowPosition(w, x, y);
		}
	}

	void VGWindow::GetWindowScreenPosition(int* outX, int* outY) const
	{
		if (outX == nullptr || outY == nullptr)
		{
			return;
		}
		*outX = 0;
		*outY = 0;
		if (SDL_Window* w = const_cast<VGWindow*>(this)->GetSDLWindow())
		{
			SDL_GetWindowPosition(w, outX, outY);
		}
	}

	void VGWindow::SetWindowResizableFlag(bool value)
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_SetWindowResizable(w, value);
		}
	}

	void VGWindow::Maximize()
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_MaximizeWindow(w);
		}
	}

	void VGWindow::Minimize()
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_MinimizeWindow(w);
		}
	}

	void VGWindow::Restore()
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_RestoreWindow(w);
		}
	}

	void VGWindow::Show()
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_ShowWindow(w);
		}
	}

	void VGWindow::Hide()
	{
		if (SDL_Window* w = GetSDLWindow())
		{
			SDL_HideWindow(w);
		}
	}
}
