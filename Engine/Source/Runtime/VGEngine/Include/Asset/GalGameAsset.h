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

namespace VisionGal
{
	struct GalGameStoryScriptAsset : public VGAsset
	{
		std::string Text;

		GalGameStoryScriptAsset()
			: VGAsset("GalGameStoryScript")
		{
		}
	};

	class VG_ENGINE_API GalGameStoryScriptAssetWriter : public IAssetWriter
	{
	public:
		GalGameStoryScriptAssetWriter() = default;
		~GalGameStoryScriptAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

}