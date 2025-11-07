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
#include "Asset.h"
#include "../Core/Core.h"
#include "../Resource/Audio.h"

namespace VisionGal
{
	struct AudioAsset : public VGAsset
	{
		AudioAsset()
			: VGAsset("Audio")
		{
		}

		Ref<AudioClip> audioClip;
	};

	class VG_ENGINE_API AudioAssetLoader : public IAssetLoader
	{
	public:
		AudioAssetLoader() = default;
		~AudioAssetLoader() override = default;
	
		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};

}
