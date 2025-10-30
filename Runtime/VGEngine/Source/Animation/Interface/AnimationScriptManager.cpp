#include "Animation/Interface/AnimationScriptManager.h"
#include "Scene/Components.h"

namespace VisionGal {


	bool AnimationScriptManager::AddActorAnimationScript(GameActor* actor, const Ref<IAnimationScript>& script)
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