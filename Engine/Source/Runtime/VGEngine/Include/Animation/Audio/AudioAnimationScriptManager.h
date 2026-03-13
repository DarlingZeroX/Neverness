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

#include "../..//Core/Core.h"
#include "../../Interface/GameInterface.h"

namespace VisionGal {

	class AudioAnimationScriptManager
	{
	public:
		static AudioAnimationScriptManager* GetInstance();

		static Ref<IAnimationScript> CreateAudioAnimationWithCommand(GameActor* actor, const String& cmd);

		static bool StartAudioAnimationWithCommand(GameActor* actor, const String& cmd);
		//static bool StartAudioTransform(GameActor* actor, const Ref<IAnimationScript>& script);
	};

}