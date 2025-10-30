#include "../Core/Core.h"
#include "../Interface/GameInterface.h"

namespace VisionGal::GalGame {

	class SpriteTransformScriptManager
	{
	public:
		static SpriteTransformScriptManager* GetInstance();

		static Ref<IAnimationScript> CreateSpriteTransformWithCommand(GameActor* actor, const String& cmd);

		static bool StartSpriteTransformWithCommand(GameActor* actor, const String& cmd);
	};

}