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

#include "../../ApplicationExport.h"
#include <SDL3/SDL.h>
#include <NNPlatformCore/Include/SDL3/SDL3Window.h>

#include <NNCore/Include/Event/HEventDelegate.h>
#include <NNCore/Include/Event/HWindowEvents.h>

#include "NNNativeEngineAPI/Include/WindowAPI.h"

namespace NN::Runtime
{
    /**
     * @brief Runtime 宿主窗口：SDL3 + OpenGL，供 Editor 与 Window API 共用。
     * SDL 子系统须由 **RuntimeApplication::Initialize** 先行初始化。
     */
    class NN_RUNTIME_APPLICATION_API VGWindow: public NN::Core::SDL3::OpenGLWindow
    {
    public:
        VGWindow();
        VGWindow(const VGWindow&) = delete;
        VGWindow& operator=(const VGWindow&) = delete;
        ~VGWindow() override = default;

        /** @brief 创建无边框 + HitTest 模式（Editor 标题栏拖拽）。 */
		void SetInitializeBorderless(bool borderless);

        /**
         * @brief 按 ABI 描述创建窗口与 OpenGL 上下文。
         * @return 成功返回 true；失败时窗口未注册。
         */
        bool CreateFromDesc(const NNWindowDesc* desc);

        /** @brief 兼容旧路径；内部转调 `CreateFromDesc`。 */
        bool Initialize(const char* window_name, int width, int height, bool allow_resize);

        SDL_GLContext GetContext() override { return m_GLContext; }

        void SetWindowTitle(const char* title);
        void SetWindowPixelSize(int width, int height);
        void GetWindowPixelSize(int* outWidth, int* outHeight) const;
        void SetWindowScreenPosition(int x, int y);
        void GetWindowScreenPosition(int* outX, int* outY) const;
        void SetWindowResizableFlag(bool value);
        void Maximize();
        void Minimize();
        void Restore();
        void Show();
        void Hide();

    private:
        SDL_GLContext m_GLContext = nullptr;

		bool m_OnResizeWindowMode = false;
		bool m_Borderless = false;
    };

}
