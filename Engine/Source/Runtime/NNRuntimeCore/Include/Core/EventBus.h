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
#include <NNCore/Include/Event/HEventDelegate.h>

namespace NN::Runtime
{
	struct VG_CORE_API EngineEventBus
	{
		//NN::Core::HEventDelegate<const TransformUpdateEvent&> OnTransformUpdateEvent;

		NN::Core::HEventDelegate<const SceneEvent&> OnSceneEvent;

		NN::Core::HEventDelegate<const EngineEvent&> OnEngineEvent;

		NN::Core::HEventDelegate<const LuaScriptEvent&> OnLuaScriptEvent;

		NN::Core::HEventDelegate<const UISystemEvent&> OnUISystemEvent;

		static EngineEventBus& Get();
	};

}