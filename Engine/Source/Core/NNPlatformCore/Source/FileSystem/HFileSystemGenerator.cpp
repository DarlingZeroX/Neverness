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

#include "HFileSystemGenerator.h"
#include <NNCore/Include/Utils/HStringGenerator.h>

namespace NN::Core
{
	fsPath HSequenceGenerator::GenerateDirectory(const fsPath& path)
	{
		HSequenceWStringGenerator gen(path);

		auto nextPath = gen.GetNext();
		while (HFileSystem::ExistsDirectory(nextPath))
		{
			nextPath = gen.GetNext();
		}

		HFileSystem::CreateDirectory(nextPath);

		return nextPath;
	}

	fsPath HSequenceGenerator::GenerateAsset(const fsPath& path, const std::wstring& name)
	{
		HSequenceWStringGenerator gen(name);

		auto nextName = gen.GetNext();
		auto fullPath = path / (nextName + L".hasset");
		while (HFileSystem::ExistsDirectory(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = path / (nextName + L".hasset");
		}

		HFileSystem::CreateNullFile(fullPath);

		return fullPath;
	}

	std::wstring HSequenceGenerator::GenerateAssetName(HPath path, std::wstring name)
	{
		HSequenceWStringGenerator gen(name);
		//auto fullPath = HVirtualFileSystem::HPathToPfsPath(path / name).wstring() + L".hasset";

		std::wstring nextName = name;
		//while (HFileSystem::FileExists(fullPath))
		//{
		//	nextName = gen.GetNext();
		//
		//	fullPath = HVirtualFileSystem::HPathToPfsPath(path / nextName).wstring() + L".hasset";
		//}

		return nextName;
	}


}
