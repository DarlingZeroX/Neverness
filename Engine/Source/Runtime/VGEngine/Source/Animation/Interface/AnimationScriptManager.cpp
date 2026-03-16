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

#include "Animation/Interface/AnimationScriptManager.h"
#include "Scene/Components.h"

namespace VisionGal {


	bool AnimationScriptManager::AddActorAnimationScript(IGameActor* actor, const Ref<IAnimationScript>& script)
	{
		if (actor == nullptr)
			return false;

		if (script == nullptr)
			return false;

		auto* com = actor->GetComponent<AnimationScriptComponent>();

		if (com == nullptr)
		{
			com = actor->AddComponent<AnimationScriptComponent>();
		}

		com->scripts.push_back(script);

		return true;
	}
}