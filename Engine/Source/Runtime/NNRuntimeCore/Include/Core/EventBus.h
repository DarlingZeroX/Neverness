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
#include "Events.h"
#include <NNKernel/Include/Event/HEventDelegate.h>

namespace VisionGal
{
	struct VG_CORE_API EngineEventBus
	{
		//Horizon::HEventDelegate<const TransformUpdateEvent&> OnTransformUpdateEvent;

		Horizon::HEventDelegate<const SceneEvent&> OnSceneEvent;

		Horizon::HEventDelegate<const EngineEvent&> OnEngineEvent;

		Horizon::HEventDelegate<const LuaScriptEvent&> OnLuaScriptEvent;

		Horizon::HEventDelegate<const UISystemEvent&> OnUISystemEvent;

		static EngineEventBus& Get();
	};

}