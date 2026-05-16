#pragma once
#include <string>
#include "../HCorePlatformConfig.h"

/**
 * \file HClipboard.h
 *
 * Include file encapsulate for SDL clipboard handling
 */

namespace NN::Core
{
	struct H_CORE_PLATFORM_API HClipboard
	{
		/**
		 * Put UTF-8 text into the clipboard.
		 *
		 * \param text the text to store in the clipboard
		 * \returns 0 on success or a negative error code on failure; call
		 *          SDL_GetError() for more information.
		 */
		static int SetText(const char* text);

		/**
		* This functions returns empty string if there was not enough memory left for
		* a copy of the clipboard's content.
		*
		* \returns the clipboard text on success or an empty string on failure; call
		*          SDL_GetError() for more information. Caller must call SDL_free()
		*          on the returned pointer when done with it (even if there was an
		*          error).
		*/
		static std::string GetText();

		/**
		* Query whether the clipboard exists and contains a non-empty text string.
		*
		* \returns true if the clipboard has text, or flase if it does not.
		*/
		static bool HasText();
	};
}
