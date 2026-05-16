/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** VariantContainer.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "VariantBase.h"

#include "../MetaTraits.h"

#include "../Object.h"
#include <cstdint>

BEGIN_META_NAMESPACE

#define DEFAULT_TYPE_HANDLER(typeName)                                                           \
    template<typename U = T>                                                                     \
    typeName get##typeName(                                                                      \
        typename std::enable_if<                                                                 \
            !std::is_convertible<typename traits::TypeOrEnumType<U>::type, typeName>::value \
        >::type* = nullptr                                                                       \
    ) const;                                                                                     \
    template<typename U = T>                                                                     \
    typeName get##typeName(                                                                      \
        typename std::enable_if<                                                                 \
            std::is_convertible<typename traits::TypeOrEnumType<U>::type, typeName>::value  \
        >::type* = nullptr                                                                       \
    ) const;                                                                                     \

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

template<typename T>
class  VariantContainer : public VariantBase
{

public:
	typedef typename std::remove_reference<T>::type NonRefType;

	VariantContainer(const NonRefType& value);
	VariantContainer(const NonRefType&& value);

	Type GetType(void) const override;

	void* GetPtr(void) const override;

	std::int8_t ToInt8() const override;
	std::int16_t ToInt16() const override;
	std::int32_t ToInt32() const override;
	std::int64_t ToInt64() const override;
	std::uint8_t ToUint8() const override;
	std::uint16_t ToUint16() const override;
	std::uint32_t ToUint32() const override;
	std::uint64_t ToUint64() const override;

	//uint32 ToUin32() const override;
	int ToInt(void) const override;
	long ToLong() const override;
	bool ToBool(void) const override;
	float ToFloat(void) const override;
	double ToDouble(void) const override;
	std::string ToString(void) const override;

	bool IsValid() const;
	VariantBase* Clone(void) const override;

	void OnSerialize(Json::object& output) const override;
	void OnDeserialize(const Json& input) override;
private:
	friend class Variant;

	T m_value;

	VariantContainer& operator=(const VariantContainer& rhs) = delete;

	DEFAULT_TYPE_HANDLER(int8_t);
	DEFAULT_TYPE_HANDLER(int16_t);
	DEFAULT_TYPE_HANDLER(int32_t);
	DEFAULT_TYPE_HANDLER(int64_t);
	DEFAULT_TYPE_HANDLER(uint8_t);
	DEFAULT_TYPE_HANDLER(uint16_t);
	DEFAULT_TYPE_HANDLER(uint32_t);
	DEFAULT_TYPE_HANDLER(uint64_t);

	DEFAULT_TYPE_HANDLER(int);
	DEFAULT_TYPE_HANDLER(long);
	DEFAULT_TYPE_HANDLER(bool);
	DEFAULT_TYPE_HANDLER(float);
	DEFAULT_TYPE_HANDLER(double);

	template<typename U = T>
	std::string getString(
		typename std::enable_if<
		!std::is_arithmetic<U>::value
		>::type* = nullptr
	) const;

	template<typename U = T>
	std::string getString(
		typename std::enable_if<
		std::is_arithmetic<U>::value
		>::type* = nullptr
	) const;

	template<typename U = T>
	void onSerialize(
		Json::object& output,
		typename std::enable_if<
		!std::is_pointer<U>::value&& std::is_base_of<Object, U>::value
		>::type* = nullptr
	) const;

	template<typename U = T>
	void onSerialize(
		Json::object& output,
		typename std::enable_if<
		std::is_pointer<U>::value || !std::is_base_of<Object, U>::value
		>::type* = nullptr
	) const;

	template<typename U = T>
	void onDeserialize(
		const Json& input,
		typename std::enable_if<
		!std::is_pointer<U>::value&& std::is_base_of<Object, U>::value
		>::type* = nullptr
	);

	template<typename U = T>
	void onDeserialize(
		const Json& input,
		typename std::enable_if<
		std::is_pointer<U>::value || !std::is_base_of<Object, U>::value
		>::type* = nullptr
	);

};

END_META_NAMESPACE

#undef DEFAULT_TYPE_HANDLER

#include "../../Impl/VariantContainer.hpp"
#include "../../Impl/VariantContainerStandardTypes.hpp"