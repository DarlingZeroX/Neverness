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
#include "../../VGCoreConfig.h"
#include <SDL3/SDL.h>
#include <HCorePlatform/Include/SDL3/SDL3Window.h>

#include <HCore/Include/Event/HEventDelegate.h>
#include <HCore/Include/Event/HWindowEvents.h>

namespace VisionGal
{
    class VG_CORE_API VGWindow: public Horizon::SDL3::OpenGLWindow
    {
    public:
        VGWindow();
        VGWindow(const VGWindow&) = delete;
        VGWindow& operator=(const VGWindow&) = delete;
        ~VGWindow() override = default;
    public:
		void SetInitializeBorderless(bool borderless);
        bool Initialize(const char* window_name, int width, int height, bool allow_resize);
        SDL_GLContext GetContext() override { return m_GLContext; }
    private:
        SDL_GLContext m_GLContext;

		bool m_OnResizeWindowMode = false;
		bool m_Borderless = false;
    };

}