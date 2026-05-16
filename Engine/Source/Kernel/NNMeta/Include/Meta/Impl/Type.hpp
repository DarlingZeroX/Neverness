/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Type.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "TypeUnpacker.hpp"

#include "../Core/Variant/Variant.h"
//#include "../ConstructorInfo.h"

//#include "../Common/Logging.h"
#include "../Deps/Logging.h"

namespace std
{
	template<>
	struct hash<META_NAMESPACE::Type>
	{
		size_t operator()(const META_NAMESPACE::Type& type) const
		{
			return hash<META_NAMESPACE::TypeID>()(type.GetID());
		}
	};
}

BEGIN_META_NAMESPACE
template<typename T>
Type Type::Get(T&& obj)
{
	return { TypeOf(T) };
}

// Converts the expression into a meta::Type instance
//#define TypeOf(expr)                              \
//    META_NAMESPACE::Type(                           \
//        TypeIDOf( expr ),                         \
//        META_NAMESPACE::traits::IsArray<expr>::value \
//    )                                             \

template <typename T>
Type Type::Get()
{
	return META_NAMESPACE::Type(
		TypeIDOf(T),
		META_NAMESPACE::traits::IsArray<T>::value
	);
}

///////////////////////////////////////////////////////////////////////

template<typename T>
bool Type::DerivesFrom(void) const
{
	return DerivesFrom(TypeOf(T));
}

template<typename ClassType>
Json Type::SerializeJson(const ClassType& instance, bool invokeHook)
{
	auto type = TypeOf(ClassType);

	UAssert(type.IsValid(),
		"Invalid type serialized."
	);

	Variant variant = instance;

	return type.SerializeJson(variant, invokeHook);
}

template<typename ClassType>
ClassType Type::DeserializeJson(const Json& value)
{
	auto type = TypeOf(ClassType);

	UAssert(type.IsValid(),
		"Invalid type created."
	);

	return type.DeserializeJson(value).GetValue<ClassType>();
}
END_META_NAMESPACE