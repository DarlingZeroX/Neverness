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

#include "VFSMount.h"
#include "PackageFileSystem.h"
#include <NNCore/Interface/HLog.h>
#include <NNRuntimeVFS/Include/VFSService.h>
#include <NNRuntimeVFS/Include/VFS/VFSNative.h>
#include <filesystem>

namespace NN::Runtime::VFS
{
	bool MountPackageFileSystem(const std::string& alias, const std::string& pakPath, const std::string& backAsbPath)
	{
		IFileSystemPtr fsPtr;
		if (std::filesystem::exists(pakPath))
		{
			fsPtr = std::make_shared<VGPackageFileSystem>(pakPath);
			H_LOG_INFO("%s使用运行时资源包: %s", alias.c_str(), pakPath.c_str());
		}
		else
		{
			fsPtr = std::make_shared<NativeFileSystem>(backAsbPath);

			if (std::filesystem::exists(backAsbPath) == false)
			{
				H_LOG_ERROR("无法挂载可用的文件系统: %s", pakPath.c_str());
				return false;
			}
		}

		return VFSService::MountFileSystem(alias, std::move(fsPtr));
	}
}
