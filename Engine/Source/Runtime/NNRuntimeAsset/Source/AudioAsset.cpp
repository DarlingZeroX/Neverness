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

#include "AudioAsset.h"
//#include <NNMediaCore/Include/>
#include "AudioClip.h"

namespace VisionGal
{
	bool AudioAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		auto videoAsset = MakeRef<AudioAsset>();
		//videoAsset->audioClip = MakeRef<AudioClip>();
		auto audioClip = MakeRef<VGAudioClip>();

		//if (videoAsset->audioClip->Open(path))
		if (audioClip->Open(path))
		{
			videoAsset->audioClip = audioClip;
			asset = videoAsset;
			return true;
		}

		return false;
	}
}