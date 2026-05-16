/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Argument.hpp
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE
template<typename T>
Argument::Argument(const T& data)
	: m_typeID(TypeIDOf(T))
	, m_isArray(traits::IsArray<T>::value)
	, m_data(reinterpret_cast<const void*>(std::addressof(data)))
{
	static_assert(!std::is_same< Argument, T >::value,
		"Cannot use Argument as an argument"
		);
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Argument::Argument(T& data)
	: m_typeID(TypeIDOf(T))
	, m_isArray(traits::IsArray<T>::value)
	, m_data(reinterpret_cast<const void*>(std::addressof(data)))
{
	static_assert(!std::is_same< Argument, T >::value,
		"Cannot use Argument as an argument"
		);
}

///////////////////////////////////////////////////////////////////////

template<typename T>
T& Argument::GetValue(void) const
{
	return *reinterpret_cast<
		typename std::remove_reference< T >::type*
	>(
		const_cast<void*>(m_data)
		);
}
END_META_NAMESPACE