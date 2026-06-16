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

namespace NN::Runtime
{
	struct GLuaScriptAssetType: public VGAssetType
	{
		std::string GetNameID() const override { return "GalGameLuaScript"; }
	};

	struct GalGameLuaScriptAsset : public VGAsset
	{
		std::string Text;

		GalGameLuaScriptAsset()
			: VGAsset(GLuaScriptAssetType{}.GetNameID())
		{
		}
	};

	class VG_ASSET_API GalGameStoryScriptAssetWriter : public IAssetWriter
	{
	public:
		GalGameStoryScriptAssetWriter() = default;
		~GalGameStoryScriptAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

}