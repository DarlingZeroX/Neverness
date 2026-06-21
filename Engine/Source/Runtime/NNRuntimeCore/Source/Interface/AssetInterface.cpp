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
#include "NNCore/Include/Utils/HStringGenerator.h"
#include "NNPlatformCore/Interface/FileSystem/HFileSystem.h"

namespace NN::Runtime
{
	VGPath GenerateAssetPath(const VGPath& path, const std::string& name, const std::string& ext)
	{
		NN::Core::HSequenceStringGenerator gen(name);

		std::filesystem::path fsPath = path;
		auto nextName = gen.GetNext();
		auto fullPath = fsPath / (nextName + ext);
		while (NN::Core::HFileSystem::ExistsDirectory(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = fsPath / (nextName + ext);
		}

		return fullPath.string();
	}
}
