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

#include "Loader.h"
#include <HCore/Interface/HTypeInfo.h>

namespace VisionGal
{
	static Horizon::unordered_type_map<Scope<VGObjectLoader>> s_ObjectLoaders;

	VGObjectPtr StaticLoadObject(const String& path, const type_info& typeInfo)
	{
		if (auto result = s_ObjectLoaders.find(&typeInfo); result != s_ObjectLoaders.end())
		{
			return result->second->StaticLoadObject(path);
		}

		return nullptr;
	}

	int RegisterObjectLoader(const type_info& typeInfo, VGObjectLoader* loader)
	{
		s_ObjectLoaders[&typeInfo] = std::unique_ptr<VGObjectLoader>(loader);

		return 0;
	}
}