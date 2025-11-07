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
#include "../Resource/Video.h"

namespace VisionGal
{
	struct VideoAsset : public VGAsset
	{
		VideoAsset()
			: VGAsset("Video")
		{
		}

		Ref<VideoClip> videoClip;
	};

	class VG_ENGINE_API VideoAssetLoader : public IAssetLoader
	{
	public:
		VideoAssetLoader() = default;
		~VideoAssetLoader() override = default;
		 
		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};
}