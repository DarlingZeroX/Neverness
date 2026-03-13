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

#include "Asset/AudioAsset.h"
//#include <HMedia/Include/>
#include "Resource/Audio.h"

namespace VisionGal
{
	bool AudioAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		auto videoAsset = CreateRef<AudioAsset>();
		//videoAsset->audioClip = CreateRef<AudioClip>();
		auto audioClip = CreateRef<VGAudioClip>();

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