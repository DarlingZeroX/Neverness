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
#include "../../EngineConfig.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include "NNRuntimeCore/Interface/GameInterface.h"

namespace VisionGal {

	class VG_ENGINE_API AnimationScriptManager
	{
	public:
		static bool AddActorAnimationScript(IGameActor* actor, const Ref<IAnimationScript>& script);
	};

}