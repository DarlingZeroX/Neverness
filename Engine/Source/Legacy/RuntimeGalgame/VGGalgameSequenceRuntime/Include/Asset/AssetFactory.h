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
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeCore/Interface/AssetInterface.h"

namespace VisionGal::GalGame
{
	struct GalGameSequenceScriptAssetFactory: public IAssetFactoryInstance
	{
		GalGameSequenceScriptAssetFactory() = default;
		~GalGameSequenceScriptAssetFactory() override = default;

		std::string GetFactoryType() override;
		Ref<VGAsset> CreateAsset(const String& path) override;
	};
}
