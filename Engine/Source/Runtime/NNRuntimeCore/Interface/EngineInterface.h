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
#include "NNRuntimeCore/Include/Core/Core.h"
#include <SDL3/SDL.h>

namespace NN::Runtime
{
	struct IEngineApplicationLayer
	{
		virtual ~IEngineApplicationLayer() = default;

		virtual void OnLayerInitialize() = 0;
		virtual void OnLayerRender() = 0;
		virtual void OnLayerUpdate(float delta) = 0;
	};

	struct IEngineApplication
	{
		virtual ~IEngineApplication() = default;

		virtual void AddApplicationLayer(IEngineApplicationLayer* layer) = 0;
		virtual void OnApplicationUpdate(float deltaTime) = 0;
		virtual int ProcessEvent(const SDL_Event& event) = 0;
		virtual void MakeCurrentRenderContext() = 0;
	};

	struct IUISystem
	{
		virtual ~IUISystem() = default;

		//virtual void* LoadUIFile(const String& path) = 0;
		//virtual bool ShowUI(void* ui) = 0;
		virtual void* GetContext() = 0;
	};


}