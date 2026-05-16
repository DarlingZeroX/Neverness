/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeInfo.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "TypeConfig.h"

#include "../../Deps/Logging.h"

#include <type_traits>

#if defined(COMPILER_CLANG) || defined(COMPILER_GNU)

#define IsTriviallyDefaultConstructible(x) std::has_trivial_default_constructor<x>::value

#else

#define IsTriviallyDefaultConstructible(x) std::is_trivially_default_constructible<x>::value

#endif

BEGIN_META_NAMESPACE
struct TypeData;

//template<typename T>
struct HMETA_API TypeInfo
{
	//static bool Defined;

	template<typename T>
	static void Register(TypeID id, TypeData& data, bool beingDefined);

	static bool TypeDefined(const type_info& info);

	static void TypeSetDefined(const type_info& info);

	template<typename  T>
	static const bool Defined()
	{
		return TypeDefined(typeid(T));
	}

	template<typename  T>
	static void SetDefined()
	{
		TypeSetDefined(typeid(T));
	}

private:
	//template<typename U = T>
	template<typename U>
	static void addDefaultConstructor(
		TypeData& data,
		typename std::enable_if<
		!IsTriviallyDefaultConstructible(U)
		>::type* = nullptr
	);

	//template<typename U = T>
	template<typename U>
	static void addDefaultConstructor(
		TypeData& data,
		typename std::enable_if<
		IsTriviallyDefaultConstructible(U)
		>::type* = nullptr
	);

	template<typename U>
	static void applyTrivialAttributes(TypeData& data,
		typename std::enable_if<
		!std::is_trivial<U>::value
		>::type* = nullptr
	);

	template<typename U>
	static void applyTrivialAttributes(TypeData& data,
		typename std::enable_if<
		std::is_trivial<U>::value
		>::type* = nullptr
	);
};
END_META_NAMESPACE

#include "../../Impl/TypeInfo.hpp"
