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

#include "Asset/Asset.h"
#include "NNRuntimeAsset/Interface/Package.h"
#include "NNRuntimeVFS/Include/VFSService.h"
#include "Sequence/DataContainerSerialization.h"

namespace VisionGal::GalGame
{
	bool SequenceScriptAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		if (asset == nullptr)
			return false;

		SequenceScriptAsset* uiAsset = dynamic_cast<SequenceScriptAsset*>(asset);

		if (uiAsset == nullptr)
			return false;

		std::string sequenceData = SerializeVGSSequenceDataContainerToString(*uiAsset->ExecutionData->SequenceData, 2);

		// 创建或打开一个文件用于写入
		if (VFSService::WriteTextToFile(path, sequenceData) == false)
			return false;

		// 写入元信息
		auto package = VGPackage::NewPackage(path);
		package->SetAsset(uiAsset);
		package->WriteMetaData("");

		return true;
	}

	bool SequenceScriptAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		std::string text;
		if (VFSService::ReadTextFromFile(path, text) == false)
			return false;

		auto sequenceData = MakeRef<VGSSequenceDataContainer>();
		DeserializeVGSSequenceDataContainerFromString(text, *sequenceData);

		auto vsAsset = MakeRef<SequenceScriptAsset>();
		vsAsset->ExecutionData->SequenceData = sequenceData;

		asset = vsAsset;
		return true;
	}
}
