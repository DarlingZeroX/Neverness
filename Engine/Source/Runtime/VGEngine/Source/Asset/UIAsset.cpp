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

#include "Asset/UIAsset.h"
#include "Asset/Package.h"
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal
{

	bool UIDocumentAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		if (asset == nullptr)
			return false;

		UIDocumentAsset* uiAsset = dynamic_cast<UIDocumentAsset*>(asset);

		if (uiAsset == nullptr)
			return false;

		// 创建或打开一个文件用于写入
		if (VFS::WriteTextToFile(path, uiAsset->Text) == false)
			return false;

		// 写入元信息
		auto package = VGPackage::NewPackage(path);
		package->SetAsset(uiAsset);
		package->WriteMetaData("");

		return true;
	}

	bool UICssAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		if (asset == nullptr)
			return false;

		UICssAsset* uiAsset = dynamic_cast<UICssAsset*>(asset);

		if (uiAsset == nullptr)
			return false;

		// 创建或打开一个文件用于写入
		if (VFS::WriteTextToFile(path, uiAsset->Text) == false)
			return false;

		// 写入元信息
		auto package = VGPackage::NewPackage(path);
		package->SetAsset(uiAsset);
		package->WriteMetaData("");

		return true;
	}
}
