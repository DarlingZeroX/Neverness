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
#include "EngineInterface.h"
#include "SceneInterface.h"
#include "../Include/Core/Core.h"
#include <HCorePlatform/Include/WindowInterface.h>

namespace VisionGal
{
	struct IGameAppContext
	{
		virtual ~IGameAppContext() = default;

		virtual IUISystem* GetUISystem() = 0;
		virtual Horizon::IWindow* GetWindow() = 0;
		virtual IScene* GetScene() = 0;
	};
}