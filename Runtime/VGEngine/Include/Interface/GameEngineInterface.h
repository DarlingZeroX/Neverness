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
#include "GameInterface.h"
#include "SceneInterface.h"
#include "../Core/Core.h"
#include "../Core/Viewport.h"
#include <HCore/Include/Platform/SDL3/SDL3Window.h>
#include "../Graphics/OpenGL/RenderTarget.h"

namespace VisionGal
{
	struct IRenderPipeline
	{
		virtual ~IRenderPipeline() = default;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
	};

	struct IGameEngine
	{
		virtual ~IGameEngine() = default;

		//virtual void Initialize();
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnRender() = 0;
	};
	 
	struct ISubGameEngine : public IGameEngine
	{
		~ISubGameEngine() override = default;
	};

	struct IGameEngineContext
	{
		virtual ~IGameEngineContext() = default;

		virtual IUISystem* GetUISystem() = 0;
		virtual Horizon::SDL3::OpenGLWindow* GetWindow() = 0;
		virtual Viewport* GetViewport() = 0;

		// 在渲染引擎设置各种渲染状态之前，添加渲染回调
		virtual void AddBeforeRenderCallback(const String& callbackID ,const std::function<void(OpenGL::RenderTarget2D*)>& callback) = 0;
		virtual void ExecuteBeforeRenderCallbacks(OpenGL::RenderTarget2D* rt) = 0;

		// 在渲染引擎渲染完成场景之后，解除各种渲染状态之后，添加渲染回调
		virtual void AddAfterRenderCallback(const String& callbackID, const std::function<void(OpenGL::RenderTarget2D*)>& callback) = 0;
		virtual void ExecuteAfterRenderCallbacks(OpenGL::RenderTarget2D* rt) = 0;
	};
}
