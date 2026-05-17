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
#include "../RuntimeCoreExport.h"
#include "../Include/Core/RuntimeCore.h"

namespace NN::Runtime
{
	using VGObjectPtr = Ref<VGObject>;

	NN_RUNTIME_CORE_API VGObjectPtr StaticLoadObject(const String& path, const type_info& typeInfo);

	template<class T>
	inline Ref<T> LoadObject(const String& path)
	{
		return std::dynamic_pointer_cast<T>(StaticLoadObject(path, typeid(T)));
		//return dynamic_cast<Ref<T>>( StaticLoadObject(uuid, typeid(T)) );
	}

	struct VGObjectLoader
	{
		virtual ~VGObjectLoader() = default;

		virtual VGObjectPtr StaticLoadObject(const String& path) = 0;
	};

	NN_RUNTIME_CORE_API int RegisterObjectLoader(const type_info& typeInfo, VGObjectLoader* loader);
}