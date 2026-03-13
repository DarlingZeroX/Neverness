// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../HConfig.h"
#include "HUUID.h"

namespace Horizon
{

	struct HObject
	{
		HObject() = default;
		HObject(const HObject&) = default;
		HObject& operator=(const HObject&) = default;
		HObject(HObject&&) noexcept = default;
		HObject& operator=(HObject&&) noexcept = default;
		virtual ~HObject() = default;

		virtual  std::string ToString() { return ""; }

		//virtual std::string GetLabel() const { return ""; }
		//virtual void Update() {};
	};

	using HObjectPtr = Ref<HObject>;

	CORE_MODULE_API HObjectPtr StaticLoadObject(const HUUID& uuid, const type_info& typeInfo);

	template<class T>
	inline Ref<T> LoadObject(const HUUID& uuid)
	{
		return std::dynamic_pointer_cast<T>(StaticLoadObject(uuid, typeid(T)));
		//return dynamic_cast<Ref<T>>( StaticLoadObject(uuid, typeid(T)) );
	}

	//template<class T>
	//inline HObjectPtr LoadObject(const HUUID& uuid)
	//{
	//	static_assert(std::is_base_of<HObject, T>::value);
	//	return StaticLoadObject(uuid, typeid(T));
	//}

	struct HObjectLoader
	{
		virtual ~HObjectLoader() = default;

		virtual  HObjectPtr StaticLoadObject(const HUUID& uuid) = 0;
	};

	CORE_MODULE_API HResult RegisterObjectLoader(const type_info& typeInfo, HObjectLoader* loader);
}