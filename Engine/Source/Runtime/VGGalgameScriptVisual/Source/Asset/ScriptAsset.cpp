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

#include "Asset/ScriptAsset.h"
#include "VGCore/Include/Core/VFS.h"

namespace VisionGal
{
	bool GalGameVisualStoryScriptAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		return false;
	}

	bool GalGameVisualStoryScriptAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		return false;
	}
}
