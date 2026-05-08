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

#include "Asset/ScriptAssetFactory.h"
#include "Asset/ScriptAsset.h"
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal
{
	std::string GalGameVisualScriptAssetFactory::GetFactoryType()
	{
		return "VisualGalGameStoryScript";
	}

	Ref<VGAsset> GalGameVisualScriptAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先得到保存路径
		auto aPath = GenerateAssetPath(absolutePath, "NewScene", ".vgasset");
		auto rPath = VFS::GetResourcePathVFS(aPath);

		// 创建场景资产
		GalGameVisualStoryScriptAssetWriter writer;
		Ref<GalGameVisualStoryScriptAsset> asset = MakeRef<GalGameVisualStoryScriptAsset>();
		asset->SequenceData = MakeRef<VGSSequenceDataContainer>();

		// 序列化场景资产到本地
		if (writer.Write(rPath, asset.get()))
		{
			return asset;
		}

		return nullptr;
	}
}
