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

#include "HClipboard.h"
#include <string>
#include <SDL3/SDL_clipboard.h>

namespace NN::Core
{
	int HClipboard::SetText(const char* text)
	{
		return SDL_SetClipboardText(text);
	}

	std::string HClipboard::GetText()
	{
		char* text = SDL_GetClipboardText();

		std::string result = text;

		// Get UTF-8 text from the clipboard, which must be freed with SDL_free().
		SDL_free(text);

		return result;
	}

	bool HClipboard::HasText()
	{
		return SDL_HasClipboardText();
	}
}