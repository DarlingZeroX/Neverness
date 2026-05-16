/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** VariantContainer.hpp
** --------------------------------------------------------------------------*/

#pragma once

//#include "../../Type.h"
#include "../Type.h"
//#include "../Common/Compiler.h"
#include "../Compiler.h"

#if defined(COMPILER_MSVC)

#pragma warning(push)

// unused template parameters
#pragma warning(disable : 4544)

// implicit conversion performance warnings
#pragma warning(disable : 4800)

// possible loss of data
#pragma warning(disable : 4244)

#endif

#define DEFAULT_TYPE_HANDLER_IMPL(typeName)                                                      \
    template<typename T>                                                                         \
    template<typename U>                                                                         \
    typeName VariantContainer<T>::get##typeName(                                                 \
        typename std::enable_if<                                                                 \
            !std::is_convertible<typename traits::TypeOrEnumType<U>::type, typeName>::value \
        >::type*                                                                                 \
        ) const                                                                                  \
    {                                                                                            \
        return typeName( );                                                                      \
    }                                                                                            \
    template<typename T>                                                                         \
    template<typename U>                                                                         \
    typeName VariantContainer<T>::get##typeName(                                                 \
        typename std::enable_if<                                                                 \
            std::is_convertible<typename traits::TypeOrEnumType<U>::type, typeName>::value  \
        >::type*                                                                                 \
    ) const                                                                                      \
    {                                                                                            \
        return static_cast<typeName>( m_value );                                                 \
    }                                                                                            \

BEGIN_META_NAMESPACE
template<typename T>
VariantContainer<T>::VariantContainer(const NonRefType& value)
	: m_value(const_cast<NonRefType&>(value))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
VariantContainer<T>::VariantContainer(const NonRefType&& value)
	: m_value(std::move(const_cast<NonRefType&&>(value)))
{
}

///////////////////////////////////////////////////////////////////////

template<typename T>
Type VariantContainer<T>::GetType(void) const
{
	return TypeOf(T);
}

///////////////////////////////////////////////////////////////////////

template<typename T>
int VariantContainer<T>::ToInt(void) const
{
	return getint();
}

template <typename T>
long VariantContainer<T>::ToLong() const
{
	return getlong();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
bool VariantContainer<T>::ToBool(void) const
{
	return getbool();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
float VariantContainer<T>::ToFloat(void) const
{
	return getfloat();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
double VariantContainer<T>::ToDouble(void) const
{
	return getdouble();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
std::string VariantContainer<T>::ToString(void) const
{
	return getString();
}

template <typename T>
bool VariantContainer<T>::IsValid() const
{
	return true;
}

///////////////////////////////////////////////////////////////////////

template<typename T>
void* VariantContainer<T>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

template <typename T>
std::int8_t VariantContainer<T>::ToInt8() const
{
	return getint8_t();
}

template <typename T>
std::int16_t VariantContainer<T>::ToInt16() const
{
	return getint16_t();
}

template <typename T>
std::int32_t VariantContainer<T>::ToInt32() const
{
	return getint32_t();
}

template <typename T>
std::int64_t VariantContainer<T>::ToInt64() const
{
	return getint64_t();
}

template <typename T>
std::uint8_t VariantContainer<T>::ToUint8() const
{
	return getuint8_t();
}

template <typename T>
std::uint16_t VariantContainer<T>::ToUint16() const
{
	return getuint16_t();
}

template <typename T>
std::uint32_t VariantContainer<T>::ToUint32() const
{
	return getuint32_t();
}

template <typename T>
std::uint64_t VariantContainer<T>::ToUint64() const
{
	return getuint64_t();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
VariantBase* VariantContainer<T>::Clone(void) const
{
	// use the non reference type to ensure a copy is made
	return new VariantContainer<NonRefType>(m_value);
}

template<typename T>
void VariantContainer<T>::OnSerialize(Json::object& output) const
{
	onSerialize(output);
}

template<typename T>
void VariantContainer<T>::OnDeserialize(const Json& input)
{
	onDeserialize(input);
}

///////////////////////////////////////////////////////////////////////

DEFAULT_TYPE_HANDLER_IMPL(int8_t);
DEFAULT_TYPE_HANDLER_IMPL(int16_t);
DEFAULT_TYPE_HANDLER_IMPL(int32_t);
DEFAULT_TYPE_HANDLER_IMPL(int64_t);
DEFAULT_TYPE_HANDLER_IMPL(uint8_t);
DEFAULT_TYPE_HANDLER_IMPL(uint16_t);
DEFAULT_TYPE_HANDLER_IMPL(uint32_t);
DEFAULT_TYPE_HANDLER_IMPL(uint64_t);

DEFAULT_TYPE_HANDLER_IMPL(int);
DEFAULT_TYPE_HANDLER_IMPL(long);
DEFAULT_TYPE_HANDLER_IMPL(bool);
DEFAULT_TYPE_HANDLER_IMPL(float);
DEFAULT_TYPE_HANDLER_IMPL(double);

template<typename T>
template<typename U>
std::string VariantContainer<T>::getString(
	typename std::enable_if<
	!std::is_arithmetic<U>::value
	>::type*
) const
{
	return std::string();
}

template<typename T>
template<typename U>
std::string VariantContainer<T>::getString(
	typename std::enable_if<
	std::is_arithmetic<U>::value
	>::type*
) const
{
	return std::to_string(m_value);
}

template<typename T>
template<typename U>
void VariantContainer<T>::onSerialize(
	Json::object& output,
	typename std::enable_if<
	!std::is_pointer<U>::value&& std::is_base_of<Object, U>::value
	>::type*
) const
{
	//m_value.OnSerialize(output);
}

template<typename T>
template<typename U>
void VariantContainer<T>::onSerialize(
	Json::object& output,
	typename std::enable_if<
	std::is_pointer<U>::value || !std::is_base_of<Object, U>::value
	>::type*
) const
{
	// do nothing
}

template<typename T>
template<typename U>
void VariantContainer<T>::onDeserialize(
	const Json& input,
	typename std::enable_if<
	!std::is_pointer<U>::value&& std::is_base_of<Object, U>::value
	>::type*
)
{
	//m_value.OnDeserialize(input);
}

template<typename T>
template<typename U>
void VariantContainer<T>::onDeserialize(
	const Json& input,
	typename std::enable_if<
	std::is_pointer<U>::value || !std::is_base_of<Object, U>::value
	>::type*
)
{
	// do nothing
}
END_META_NAMESPACE

#undef DEFAULT_TYPE_HANDLER_IMPL

#if defined(COMPILER_MSVC)

#pragma warning(pop)

#endif