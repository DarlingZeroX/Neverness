#include "../../Core/Core.h"
#include "../../Interface/GameInterface.h"

namespace VisionGal {

	class AnimationScriptManager
	{
	public:
		static bool AddActorAnimationScript(GameActor* actor, const Ref<IAnimationScript>& script);
	};

}