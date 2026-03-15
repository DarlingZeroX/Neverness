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
#include "../EngineConfig.h"
#include "VGCore/Interface/VGAsset.h"

namespace VisionGal
{
	struct UIDocumentAsset : public VGAsset
	{
		std::string Text;

		UIDocumentAsset()
			: VGAsset("HTML")
			//: HAssetBase(HAssetType::Scene)
		{
		}
	};

	class VG_ENGINE_API UIDocumentAssetWriter : public IAssetWriter
	{
	public:
		UIDocumentAssetWriter() = default;
		~UIDocumentAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

	struct UICssAsset : public VGAsset
	{
		std::string Text;

		UICssAsset()
			: VGAsset("CSS")
			//: HAssetBase(HAssetType::Scene)
		{
		}
	};

	class VG_ENGINE_API UICssAssetWriter : public IAssetWriter
	{
	public:
		UICssAssetWriter() = default;
		~UICssAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};
}