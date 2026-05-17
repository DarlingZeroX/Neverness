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
#include "../VGAssetConfig.h"
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
//#include "../Resource/Audio.h"
//#include <NNMediaCore/Interface/AudioInterface.h>
#include "../Interface/AudioClip.h"

namespace NN::Runtime
{
	struct AudioAsset : public VGAsset
	{
		AudioAsset()
			: VGAsset("Audio")
		{
		}

		Ref<IAudioClip> audioClip;
	};

	class VG_ASSET_API AudioAssetLoader : public IAssetLoader
	{
	public:
		AudioAssetLoader() = default;
		~AudioAssetLoader() override = default;
	
		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};

}
