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
#include "../../GSSExport.h"
#include "../SequenceExecutionData.h"
#include "VGCore/Interface/VGAsset.h"

namespace VisionGal::GalGame
{
	struct SequenceScriptAssetType: public VGAssetType
	{
		std::string GetNameID() const override { return "GalGameSequenceScript"; }
	};

	struct SequenceScriptAsset : public VGAsset
	{
		//Ref<VGSSequenceDataContainer> SequenceData;
		Ref<SSSequenceExecutionData> ExecutionData;

		SequenceScriptAsset()
			: VGAsset(SequenceScriptAssetType{})
		{
			ExecutionData = MakeRef<SSSequenceExecutionData>();
		}
	};

	class VG_GSS_API SequenceScriptAssetWriter : public IAssetWriter
	{
	public:
		SequenceScriptAssetWriter() = default;
		~SequenceScriptAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

	class VG_GSS_API SequenceScriptAssetLoader : public IAssetLoader
	{
	public:
		SequenceScriptAssetLoader() = default;
		~SequenceScriptAssetLoader() override = default;

		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};
}