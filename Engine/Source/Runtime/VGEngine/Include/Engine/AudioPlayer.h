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
#include "../EngineConfig.h"
#include "VGCore/Include/Core/Core.h"
#include <HMedia/Include/Audio.h>
#include <VGAsset/Interface/AudioClip.h>

namespace VisionGal
{
	struct VG_ENGINE_API VGAudioPlayer: public Horizon::AudioPlayer
	{
		VGAudioPlayer() = default;
		~VGAudioPlayer() override = default;
	};
}
