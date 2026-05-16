#pragma once
#include "../../MetaCommonDefinitions.h"
#include "VariantBase.h"
#include "VariantPolicy.h"

#include "../Array/Array.h"
#include "../Type/TypeConfig.h"

#include "../Object.h"

BEGIN_META_NAMESPACE

class Argument;
class ArrayWrapper;
class Variant;

template<typename T>
using DISABLE_VARIANT = typename std::enable_if<
	!std::is_same<Variant, T>::value
>::type*;

template<typename T>
using DISABLE_ARGUMENT = typename std::enable_if<
	!std::is_same<Argument, T>::value
>::type*;

template<typename T>
using DISABLE_CONST = typename std::enable_if<
	!std::is_const<T>::value
>::type*;

class HMETA_API Variant
{
public:
	Variant(void) META_NOEXCEPT;

	// Given a type that inherits from a meta::Object and
	// supplied with the "WrapObject" policy, use the object wrapper
	// variant base
	template<typename T>
	Variant(
		T* data,
		VariantPolicy::WrapObject,
		typename std::enable_if<
		std::is_base_of<Object, T>::value
		>::type* = nullptr
	);

	template<typename T>
	Variant(T& data, DISABLE_VARIANT<T> = nullptr);

	template<typename T>
	Variant(T& data, VariantPolicy::NoCopy, DISABLE_VARIANT<T> = nullptr);

	// non-const r-value references, excluding other variants and arguments
	template<typename T>
	Variant(T&& data,
		DISABLE_VARIANT<T> = nullptr,
		DISABLE_ARGUMENT<T> = nullptr,
		DISABLE_CONST<T> = nullptr
	);

	// array types (non-const)
	template<typename T>
	Variant(Array<T>& rhs);

	// array types (const)
	template<typename T>
	Variant(const Array<T>& rhs);

	// r-value array types (non-const)
	template<typename T>
	Variant(Array<T>&& rhs);

	// r-value array types (const)
	template<typename T>
	Variant(const Array<T>&& rhs);

	// copy constructor
	Variant(const Variant& rhs);

	Variant(Variant& rhs);

	// r-value
	Variant(Variant&& rhs);

	~Variant(void);
public:
	template<typename T>
	Variant& operator=(T&& rhs);
	Variant& operator=(const Variant& rhs);
	Variant& operator=(Variant&& rhs);

	explicit operator bool(void) const;
	explicit operator char(void) const;
	explicit operator int(void) const;
	explicit operator unsigned(void) const;
	explicit operator float(void) const;
	explicit operator double(void) const;
	explicit operator std::string(void) const;

	Type GetType(void) const;
	ArrayWrapper GetArray(void) const;

	void Swap(Variant& other);

	std::int8_t ToInt8() const;
	std::int16_t ToInt16() const;
	std::int32_t ToInt32() const;
	std::int64_t ToInt64() const;
	std::uint8_t ToUint8() const;
	std::uint16_t ToUint16() const;
	std::uint32_t ToUint32() const;
	std::uint64_t ToUint64() const;

	int ToInt(void) const;
	long ToLong(void) const;
	bool ToBool(void) const;
	float ToFloat(void) const;
	double ToDouble(void) const;
	std::string ToString(void) const;

	Json SerializeJson(void) const;

	template<typename T>
	T& GetValue(void) const;

	bool IsValid(void) const;
	bool IsConst(void) const;
	bool IsArray(void) const;

private:
	friend class Type;
	friend class Argument;
	friend class DestructorInfo;

	void* getPtr(void) const;

	bool m_isConst;

	bool m_isValid;

	VariantBase* m_pContainer;
};

END_META_NAMESPACE

#include "../../Impl/Variant.hpp"
