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

#pragma once

#include "Config.h"
#include <string>

namespace NN::Runtime::VFS
{
	VG_PACKAGE_API bool MountPackageFileSystem(
		const std::string& alias,
		const std::string& pakPath,
		const std::string& backAsbPath);
}
