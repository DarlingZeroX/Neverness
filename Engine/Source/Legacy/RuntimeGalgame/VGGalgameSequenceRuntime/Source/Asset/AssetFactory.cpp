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

#include "Asset/AssetFactory.h"
#include "Asset/Asset.h"
#include "NNRuntimeVFS/Include/VFSService.h"

namespace VisionGal::GalGame
{
	std::string GalGameSequenceScriptAssetFactory::GetFactoryType()
	{
		return SequenceScriptAssetType{}.GetNameID();
	}

	Ref<VGAsset> GalGameSequenceScriptAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFSService::GetInstance()->AbsolutePath(path);

		// 先得到保存路径
		auto aPath = GenerateAssetPath(absolutePath, "GalGameVisualScript", ".vgasset");
		auto rPath = RuntimeCore::GetResourcePathVFS(aPath);

		// 创建场景资产
		SequenceScriptAssetWriter writer;
		Ref<SequenceScriptAsset> asset = MakeRef<SequenceScriptAsset>();
		AddVGSSequenceDataContainerDefaultEntries(*asset->ExecutionData->SequenceData);

		// 序列化场景资产到本地
		if (writer.Write(rPath, asset.get()))
		{
			return asset;
		}

		return nullptr;
	}
}
