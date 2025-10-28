#include "../..//Core/Core.h"
#include "../../Interface/GameInterface.h"

namespace VisionGal {

	class AudioAnimationScriptManager
	{
	public:
		static AudioAnimationScriptManager* GetInstance();

		static Ref<IAnimationScript> CreateAudioTransformWithCommand(GameActor* actor, const String& cmd);

		static bool StartAudioTransformWithCommand(GameActor* actor, const String& cmd);
		static bool StartAudioTransform(GameActor* actor, const Ref<IAnimationScript>& script);
	};

}