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

//
#include "HMessageBox.h"
#if PLATFORM_WIN32
#pragma warning(push)
#pragma warning(disable : 4005)
#define NOMINMAX
#define NODRAWTEXT
//#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>
#endif

#include <SDL3/SDL_messagebox.h>

namespace Horizon
{
	int HMessageBox::Show(const HMessageBoxData* messageboxdata, int* buttonid)
	{
		SDL_MessageBoxData nativeData;
		nativeData.buttons = reinterpret_cast<const SDL_MessageBoxButtonData*>(messageboxdata->buttons);
		nativeData.flags = messageboxdata->flags;
		nativeData.window = nullptr;
		nativeData.title = messageboxdata->title;
		nativeData.message = messageboxdata->message;
		nativeData.numbuttons = messageboxdata->numbuttons;
		nativeData.colorScheme = reinterpret_cast<const SDL_MessageBoxColorScheme*>(messageboxdata->colorScheme);

		return SDL_ShowMessageBox(&nativeData, buttonid);
	}

	int HMessageBox::ShowSimple(uint32 flags, const char* title, const char* message)
	{
#if PLATFORM_WIN32
		return MessageBoxA(nullptr, message, title, MB_OK | MB_ICONEXCLAMATION);
#else
		return SDL_ShowSimpleMessageBox(flags, title, message, nullptr);
#endif
	}

	int HMessageBox::ShowSimple(uint32 flags, const char* title, const std::string& message)
	{
		return ShowSimple(flags, title, message.c_str());
	}

	int HMessageBox::ShowSimple(uint32 flags, const std::string& title, const std::string& message)
	{
		return ShowSimple(flags, title.c_str(), message.c_str());
	}
}