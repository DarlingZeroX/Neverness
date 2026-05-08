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
#include "../../VGGalScriptVisualConfig.h"
#include "VGCore/Interface/VGAsset.h"
#include "../VisualSequence/SequenceDataContainer.h"

namespace VisionGal
{
	struct GalGameVisualStoryScriptAsset : public VGAsset
	{
		Ref<VGSSequenceDataContainer> SequenceData;

		GalGameVisualStoryScriptAsset()
			: VGAsset("VisualGalGameStoryScript")
		{
		}
	};

	class VG_GALGAME_VISUAL_SCRIPT_API GalGameVisualStoryScriptAssetWriter : public IAssetWriter
	{
	public:
		GalGameVisualStoryScriptAssetWriter() = default;
		~GalGameVisualStoryScriptAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

	class VG_GALGAME_VISUAL_SCRIPT_API GalGameVisualStoryScriptAssetLoader : public IAssetLoader
	{
	public:
		GalGameVisualStoryScriptAssetLoader() = default;
		~GalGameVisualStoryScriptAssetLoader() override = default;

		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};
}