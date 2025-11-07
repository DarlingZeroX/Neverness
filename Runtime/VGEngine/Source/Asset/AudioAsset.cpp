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

namespace VisionGal
{
	bool AudioAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		auto videoAsset = CreateRef<AudioAsset>();
		videoAsset->audioClip = CreateRef<AudioClip>();

		if (videoAsset->audioClip->Open(path))
		{
			asset = videoAsset;
			return true;
		}

		return false;
	}
}