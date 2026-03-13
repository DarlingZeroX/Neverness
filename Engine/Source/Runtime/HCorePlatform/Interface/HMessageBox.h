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

#pragma once
#include "../HCorePlatformConfig.h"
#include <string>
#include <HCore/Include/Core/HCoreTypes.h>

/**
 * SDL_MessageBox flags. If supported will display warning icon, etc.
 */

namespace Horizon
{
	/**
	* SDL_MessageBox flags. If supported will display warning icon, etc.
	*/
	enum HMessageBoxFlags
	{
		H_MESSAGEBOX_ERROR = 0x00000010,   /**< error dialog */
		H_MESSAGEBOX_WARNING = 0x00000020,   /**< warning dialog */
		H_MESSAGEBOX_INFORMATION = 0x00000040,   /**< informational dialog */
		H_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,   /**< buttons placed left to right */
		H_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100    /**< buttons placed right to left */
	};

	/**
	 * Flags for SDL_MessageBoxButtonData.
	 */
	enum HMessageBoxButtonFlags
	{
		H_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT = 0x00000001,  /**< Marks the default button when return is hit */
		H_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT = 0x00000002   /**< Marks the default button when escape is hit */
	};

	/**
	 * Individual button data.
	 */
	struct HMessageBoxButtonData
	{
		uint32 flags;       /**< ::SDL_MessageBoxButtonFlags */
		int buttonid;       /**< User defined button id (value returned via SDL_ShowMessageBox) */
		const char* text;  /**< The UTF-8 button text */
	};

	/**
	 * RGB value used in a message box color scheme
	 */
	struct HMessageBoxColor
	{
		uint8 r, g, b;
	};

	enum HMessageBoxColorType
	{
		H_MESSAGEBOX_COLOR_BACKGROUND,
		H_MESSAGEBOX_COLOR_TEXT,
		H_MESSAGEBOX_COLOR_BUTTON_BORDER,
		H_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
		H_MESSAGEBOX_COLOR_BUTTON_SELECTED,
		H_MESSAGEBOX_COLOR_MAX
	};

	/**
	 * A set of colors to use for message box dialogs
	 */
	struct HMessageBoxColorScheme
	{
		HMessageBoxColor colors[H_MESSAGEBOX_COLOR_MAX];
	};

	/**
	* MessageBox structure containing title, text, window, etc.
	*/
	struct HMessageBoxData
	{
		uint32 flags;                       /**< ::SDL_MessageBoxFlags */
		//SDL_Window* window;                 /**< Parent window, can be NULL */
		const char* title;                  /**< UTF-8 title */
		const char* message;                /**< UTF-8 message text */

		int numbuttons;
		const HMessageBoxButtonData* buttons;

		const HMessageBoxColorScheme* colorScheme;   /**< ::SDL_MessageBoxColorScheme, can be NULL to use system settings */
	};

	struct H_CORE_PLATFORM_API HMessageBox
	{
		/**
		* Create a modal message box.
		*
		* If your needs aren't complex, it might be easier to use
		* SDL_ShowSimpleMessageBox.
		*
		* This function should be called on the thread that created the parent
		* window, or on the main thread if the messagebox has no parent. It will
		* block execution of that thread until the user clicks a button or closes the
		* messagebox.
		*
		* This function may be called at any time, even before SDL_Init(). This makes
		* it useful for reporting errors like a failure to create a renderer or
		* OpenGL context.
		*
		* On X11, SDL rolls its own dialog box with X11 primitives instead of a
		* formal toolkit like GTK+ or Qt.
		*
		* Note that if SDL_Init() would fail because there isn't any available video
		* target, this function is likely to fail for the same reasons. If this is a
		* concern, check the return value from this function and fall back to writing
		* to stderr if you can.
		*
		* \param messageboxdata the SDL_MessageBoxData structure with title, text and
		*                       other options
		* \param buttonid the pointer to which user id of hit button should be copied
		* \returns 0 on success or a negative error code on failure; call
		*          SDL_GetError() for more information.
		*/
		static int Show(const HMessageBoxData* messageboxdata, int* buttonid);

		/**
		 * Display a simple modal message box.
		 *
		 * If your needs aren't complex, this function is preferred over
		 * SDL_ShowMessageBox.
		 *
		 * `flags` may be any of the following:
		 *
		 * - `SDL_MESSAGEBOX_ERROR`: error dialog
		 * - `SDL_MESSAGEBOX_WARNING`: warning dialog
		 * - `SDL_MESSAGEBOX_INFORMATION`: informational dialog
		 *
		 * This function should be called on the thread that created the parent
		 * window, or on the main thread if the messagebox has no parent. It will
		 * block execution of that thread until the user clicks a button or closes the
		 * messagebox.
		 *
		 * This function may be called at any time, even before SDL_Init(). This makes
		 * it useful for reporting errors like a failure to create a renderer or
		 * OpenGL context.
		 *
		 * On X11, SDL rolls its own dialog box with X11 primitives instead of a
		 * formal toolkit like GTK+ or Qt.
		 *
		 * Note that if SDL_Init() would fail because there isn't any available video
		 * target, this function is likely to fail for the same reasons. If this is a
		 * concern, check the return value from this function and fall back to writing
		 * to stderr if you can.
		 *
		 * \param flags an SDL_MessageBoxFlags value
		 * \param title UTF-8 title text
		 * \param message UTF-8 message text
		 * \param window the parent window, or NULL for no parent
		 * \returns 0 on success or a negative error code on failure; call
		 *          SDL_GetError() for more information.
		 */
		static int ShowSimple(uint32 flags, const char* title, const char* message);
		static int ShowSimple(uint32 flags, const char* title, const std::string& message);

		static int ShowSimple(uint32 flags, const std::string& title, const std::string& message);
		//static int SDL_ShowSimpleMessageBox(uint32 flags, const char* title, const char* message, SDL_Window* window);
	};
}
