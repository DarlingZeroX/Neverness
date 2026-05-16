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

#include "AssetInterface.h"
#include "NNKernel/Include/Utils/HStringGenerator.h"
#include "NNFileSystem/Interface/HFileSystem.h"

namespace VisionGal
{
	VGPath GenerateAssetPath(const VGPath& path, const std::string& name, const std::string& ext)
	{
		Horizon::HSequenceStringGenerator gen(name);

		std::filesystem::path fsPath = path;
		auto nextName = gen.GetNext();
		auto fullPath = fsPath / (nextName + ext);
		while (Horizon::HFileSystem::ExistsDirectory(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = fsPath / (nextName + ext);
		}

		return fullPath.string();
	}
}
