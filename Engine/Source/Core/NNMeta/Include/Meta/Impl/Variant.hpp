/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Variant.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Core/Variant/VariantContainer.h"
#include "../Core/ObjectWrapper.h"
#include "../Core/Array/ArrayVariantContainer.h"

BEGIN_META_NAMESPACE
template <typename T>
Variant::Variant(
	T* data,
	VariantPolicy::WrapObject,
	typename std::enable_if<
	std::is_base_of<Object, T>::value
	>::type*
)
	: m_isConst(std::is_const<T>::value)
	, m_isValid(true)
	, m_pContainer(
		new ObjectWrapper(
			static_cast<Object*>(
				const_cast<typename std::remove_const<T>::type*>(
					data
					)
				)
		)
	)
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(
	T& data,
	DISABLE_VARIANT<T>
)
	: m_isConst(std::is_pointer<T>::value&& std::is_const<T>::value)
	, m_isValid(true)
	, m_pContainer(new VariantContainer< CleanedType<T> >(data))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(
	T& data,
	VariantPolicy::NoCopy,
	DISABLE_VARIANT<T>
)
	: m_isConst(std::is_pointer<T>::value&& std::is_const<T>::value)
	, m_isValid(true)
	, m_pContainer(new VariantContainer< CleanedType<T>& >(data))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(
	T&& data,
	DISABLE_VARIANT<T>,
	DISABLE_ARGUMENT<T>,
	DISABLE_CONST<T>
)
	: m_isConst(false)
	, m_isValid(true)
	, m_pContainer(
		new VariantContainer< CleanedType<T> >(
			static_cast<T&&>(data)
			)
	)
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(Array<T>& rhs)
	: m_isConst(false)
	, m_isValid(true)
	, m_pContainer(new ArrayVariantContainer<T, traits::ArrayByReference<T>>(rhs))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(const Array<T>& rhs)
	: m_isConst(true)
	, m_isValid(true)
	, m_pContainer(new ArrayVariantContainer<T, const traits::ArrayByReference<T>>(const_cast<const traits::ArrayByReference<T>>(rhs)))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(Array<T>&& rhs)
	: m_isConst(false)
	, m_isValid(true)
	, m_pContainer(new ArrayVariantContainer<T, traits::ArrayByValue<T>>(rhs))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant::Variant(const Array<T>&& rhs)
	: m_isConst(true)
	, m_isValid(true)
	, m_pContainer(new ArrayVariantContainer<T, const traits::ArrayByValue<T>>(rhs))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Variant& Variant::operator=(T&& rhs)
{
	Variant(static_cast<T&&>(rhs)).Swap(*this);

	return *this;
}

///////////////////////////////////////////////////////////////////////

template<typename T>
T& Variant::GetValue(void) const
{
	return *static_cast<T*>(getPtr());
}
END_META_NAMESPACE