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

#include "GalGameAsset.h"
#include "NNRuntimeAsset/Interface/Package.h"
#include "NNRuntimeVFS/Include/VFSService.h"

namespace NN::Runtime
{
	bool GalGameStoryScriptAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		if (asset == nullptr)
			return false;

		GalGameLuaScriptAsset* uiAsset = dynamic_cast<GalGameLuaScriptAsset*>(asset);

		if (uiAsset == nullptr)
			return false;

		// 创建或打开一个文件用于写入
		if (VFS::VFSService::WriteTextToFile(path, uiAsset->Text) == false)
			return false;

		// 写入元信息
		auto package = VGPackage::NewPackage(path);
		package->SetAsset(uiAsset);
		package->WriteMetaData("");

		return true;
	}
}
