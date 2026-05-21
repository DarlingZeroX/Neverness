/**
 * @file BuildWindowApi.cpp
 * @brief 导出 **NNWindowAPI** 函数表（进程内 **WindowRegistry** + **VGWindow**）。
 */

#include "WindowApiExport.h"

#include "Core/WindowRegistry.h"
#include "RuntimeApplicationInstance.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/WindowAPI.h"

#include <SDL3/SDL.h>

namespace
{

void* ResolveNativeWindow(SDL_Window* w)
{
	if (w == nullptr)
	{
		return nullptr;
	}
	SDL_PropertiesID props = SDL_GetWindowProperties(w);
#if defined(_WIN32)
	return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__)
	return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#else
	return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_WINDOW_POINTER, nullptr);
#endif
}

NNWindowHandle NN_ENGINE_ABI_STDCALL WinCreate(const NNWindowDesc* desc)
{
	const NNWindowHandle handle = NN::Runtime::WindowRegistry::Create(desc);
	if (handle != NN_INVALID_WINDOW_HANDLE)
	{
		NN::Runtime::Application::GetRuntimeApplicationInstance().OnPrimaryWindowCreated(handle);
	}
	return handle;
}

void NN_ENGINE_ABI_STDCALL WinDestroy(NNWindowHandle handle)
{
	NN::Runtime::WindowRegistry::Destroy(handle);
}

void NN_ENGINE_ABI_STDCALL WinSetTitle(NNWindowHandle handle, const char* title)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->SetWindowTitle(title);
	}
}

void NN_ENGINE_ABI_STDCALL WinSetSize(NNWindowHandle handle, int width, int height)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->SetWindowPixelSize(width, height);
	}
}

void NN_ENGINE_ABI_STDCALL WinGetSize(NNWindowHandle handle, int* outWidth, int* outHeight)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->GetWindowPixelSize(outWidth, outHeight);
	}
}

void NN_ENGINE_ABI_STDCALL WinSetPosition(NNWindowHandle handle, int x, int y)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->SetWindowScreenPosition(x, y);
	}
}

void NN_ENGINE_ABI_STDCALL WinGetPosition(NNWindowHandle handle, int* outX, int* outY)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->GetWindowScreenPosition(outX, outY);
	}
}

void NN_ENGINE_ABI_STDCALL WinSetResizable(NNWindowHandle handle, bool value)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->SetWindowResizableFlag(value);
	}
}

void NN_ENGINE_ABI_STDCALL WinMaximize(NNWindowHandle handle)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->Maximize();
	}
}

void NN_ENGINE_ABI_STDCALL WinMinimize(NNWindowHandle handle)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->Minimize();
	}
}

void NN_ENGINE_ABI_STDCALL WinRestore(NNWindowHandle handle)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->Restore();
	}
}

void NN_ENGINE_ABI_STDCALL WinShow(NNWindowHandle handle)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->Show();
	}
}

void NN_ENGINE_ABI_STDCALL WinHide(NNWindowHandle handle)
{
	if (NN::Runtime::VGWindow* w = NN::Runtime::WindowRegistry::Resolve(handle))
	{
		w->Hide();
	}
}

void* NN_ENGINE_ABI_STDCALL WinGetNativeHandle(NNWindowHandle handle)
{
	return ResolveNativeWindow(NN::Runtime::WindowRegistry::TryGetSdlWindow(handle));
}

} // namespace

extern "C" void NNBuildWindowRuntimeApi(NNWindowAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	NNWindowAPI built{};
	built.size = static_cast<std::uint32_t>(sizeof(NNWindowAPI));
	built.create = &WinCreate;
	built.destroy = &WinDestroy;
	built.setTitle = &WinSetTitle;
	built.setSize = &WinSetSize;
	built.getSize = &WinGetSize;
	built.setPosition = &WinSetPosition;
	built.getPosition = &WinGetPosition;
	built.setResizable = &WinSetResizable;
	built.maximize = &WinMaximize;
	built.minimize = &WinMinimize;
	built.restore = &WinRestore;
	built.show = &WinShow;
	built.hide = &WinHide;
	built.getNativeHandle = &WinGetNativeHandle;
	*api = built;
}
