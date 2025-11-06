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
#include "../HConfig.h"
#include "../Core/HCoreTypes.h"
#include "../Event/HEventDelegate.h"
#include "../Event/HWindowEvents.h"

H_BEGIN_ROOT_NAMESPACE
	struct IWindow
{
	virtual ~IWindow() = default;

	virtual std::string GetTitle() const noexcept = 0;
	virtual void SetTitle(const std::string& title) = 0;

	virtual void* GetNativeWindow() noexcept = 0;
	virtual EWindow GetWindowType() const noexcept = 0;

	virtual unsigned int WindowWidth() const noexcept = 0;
	virtual unsigned int WindowHeight() const noexcept = 0;

	virtual void SetWindowPos(int x, int y) = 0;
	virtual uint2 GetWindowPos() const = 0;

	virtual void MinimizeWindow() = 0;
	virtual void MaximizeWindow() = 0;
	virtual void RestoreWindow() = 0;
	virtual void SetFullscreen(bool fullscreen) = 0;

	virtual int ProcessMessages() noexcept = 0;
	virtual bool SwapWindow() = 0;
	virtual void Close() = 0;
	virtual void Clear() noexcept = 0;

	using WindowEventListener = std::function<void(const Events::HWindowEvent&)>;
	HEventDelegate<const Events::HWindowEvent&> OnWindowEvent;
	virtual void AddWindowEventListener(WindowEventListener listener) = 0;
	virtual bool IsCurrentWindowEvent(unsigned int windowID) = 0;
};

H_END_NAMESPACE
