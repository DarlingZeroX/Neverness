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
#include "../CoreModuleDefinitions.h"
#include "../Platform/WindowInterface.h"
#include "../Event/HEventDelegate.h"
#include "../Event/HWindowEvents.h"

namespace Horizon
{
	class CORE_MODULE_API HWindow
	{
	public:
		HWindow(int width, int height);
		HWindow(const HWindow&) = delete;
		HWindow& operator=(const HWindow&) = delete;
		~HWindow() = default;
	public:
		std::string GetTitle() const noexcept;

		void SetTitle(const std::string& title);

		/**
		* Returns the pointer to the wrap window structure
		*/
		IWindow* GetWrapWindow() noexcept;

		/**
		* Returns the void pointer to the native window
		*/
		void* GetNativeWindow() noexcept;

		EWindow GetWindowType() const noexcept;

		/**
		* Returns the width of the window.
		* @return The width of the window.
		*/
		uint32 GetWidth() const noexcept;

		/**
		* Returns the height of the window.
		* @return The height of the window.
		*/
		uint32 GetHeight() const noexcept;

		/**
		* Resets the position of the window to the given screen coordinates.
		* @param x The x position of the window on the screen in pixels
		* @param y The y position of the window on the screen in pixels
		*/
		void SetPosition(uint32 x, uint32 y);

		/**
		* Returns the position of the window.
		* @return An 2-component integer vector.
		*/
		uint2 GetPosition() const;

		/**
		* Minimizes the window.
		*/
		void MinimizeWindow();

		/**
		* Maximizes the window.
		*/
		void MaximizeWindow();

		/**
		* Restore the window.
		*/
		void RestoreWindow();

		/**
		* Changes the window mode depending on the parameter.
		* @param fullscreen True to make the window fullscreen, false
		* to change into windowed mode.
		*/
		void SetFullscreen(bool fullscreen);

		void Close();

		int ProcessMessages() noexcept;

		void Clear() noexcept;

		HEventDelegate<const Events::HWindowEvent&> OnWindowEvent;
	private:
		Scope<IWindow> m_pWindow;
	};
}
