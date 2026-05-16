/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeInfo.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Compiler.h"
#include "../Core/Type/TypeData.h"
#include "../Core/Type/TypeID.h"

#if defined(COMPILER_MSVC)

#pragma warning(push)

// unused template parameters
#pragma warning(disable : 4544)

#endif

BEGIN_META_NAMESPACE
//template<typename T>
//bool TypeInfo<T>::Defined = false;

//template<typename T>
//struct IsBooleanType
//{
//	static constexpr bool value = false;
//};
//
//template<>
//struct IsBooleanType<bool>
//{
//	static constexpr bool value = true;
//};

template<typename T>
void TypeInfo::Register(
	TypeID id,
	TypeData& data,
	bool beingDefined
)
{
	// already defined
	if (id == Type::Invalid().GetID())
		return;

	//TypeIDs<T>::ID = id;

	typedef typename std::remove_pointer< typename std::decay<T>::type >::type Decayed;

	data.isClass = std::is_class< Decayed >::value;
	data.isEnum = std::is_enum< Decayed >::value;
	data.isPointer = std::is_pointer< T >::value;
	data.isPrimitive = std::is_arithmetic< Decayed >::value;
	data.isFloatingPoint = std::is_floating_point< Decayed >::value;
	data.isSigned = std::is_signed< Decayed >::value;

	data.typeInfo = &typeid(T);

	if (beingDefined)
	{
		addDefaultConstructor<T>(data);

		applyTrivialAttributes<T>(data);
	}
}

template<typename U>
void TypeInfo::addDefaultConstructor(
	TypeData& data,
	typename std::enable_if< !IsTriviallyDefaultConstructible(U)
	>::type*
)
{
	// do nothing
}

template<typename U>
void TypeInfo::addDefaultConstructor(
	TypeData& data,
	typename std::enable_if<
	IsTriviallyDefaultConstructible(U)
	>::type*
)
{
	// add the good 'ol default constructor
	data.AddConstructor<U, false, false>({ });

	// add the good 'ol dynamic default constructor
	data.AddConstructor<U, true, false>({ });
}

template<typename U>
void TypeInfo::applyTrivialAttributes(
	TypeData& data,
	typename std::enable_if<
	!std::is_trivial<U>::value
	>::type*
)
{
	// do nothing
}

template<typename U>
void TypeInfo::applyTrivialAttributes(
	TypeData& data,
	typename std::enable_if<
	std::is_trivial<U>::value
	>::type*
)
{
	data.SetDestructor<U>();
}
END_META_NAMESPACE

#if defined(COMPILER_MSVC)

#pragma warning(pop)

#endif