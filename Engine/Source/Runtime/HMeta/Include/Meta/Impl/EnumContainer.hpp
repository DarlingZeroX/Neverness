/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** EnumContainer.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Core/NullObject.h"

BEGIN_META_NAMESPACE
template<typename EnumType>
EnumContainer<EnumType>::EnumContainer(
	const std::string& name,
	const Initializer& initializer
)
	: EnumBase(name, InvalidTypeID)
{
	Initialize(name, initializer);
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
EnumContainer<EnumType>::EnumContainer(
	const std::string& name,
	const Initializer& initializer,
	TypeID owner
)
	: EnumBase(name, owner)
{
	Initialize(name, initializer);
}

template<typename EnumType>
void EnumContainer<EnumType>::Initialize(
	const std::string& name,
	const Initializer& initializer
)
{
	m_Keys.reserve(initializer.size());
	m_Values.reserve(initializer.size());

	for (auto& [name, value, metadata] : initializer)
	{
		m_Keys.emplace_back(name);

		m_Values.emplace_back(value);

		m_KeyToValue[name] = value;

		m_KeyToMeta[name] = metadata;
	}

	//constexpr unsigned KEY = 0;
	//constexpr unsigned VALUE = 1;
	//constexpr unsigned METADATA = 2;
	//for (auto& tp : initializer)
	//{
	//	m_Keys.emplace_back(std::get<KEY>(tp));
	//	m_Values.emplace_back(std::get<VALUE>(tp));
	//
	//	m_KeyToValue[std::get<KEY>(tp)] = std::get<VALUE>(tp);
	//
	//	m_KeyToMeta[std::get<KEY>(tp)] = std::get<METADATA>(tp);
	//}
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
Type EnumContainer<EnumType>::GetType(void) const META_NOEXCEPT
{
	return TypeOf(EnumType);
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
Type EnumContainer<EnumType>::GetUnderlyingType(void) const META_NOEXCEPT
{
	return TypeOf(typename std::underlying_type<EnumType>::type);
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
const std::vector<std::string_view>& EnumContainer<EnumType>::GetKeys(void) const META_NOEXCEPT
{
	return m_Keys;
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
const std::vector<Variant>& EnumContainer<EnumType>::GetValues(void) const META_NOEXCEPT
{
	return m_Values;

	//std::vector<Variant> values;
	//values.reserve(m_KeyToValue.size());
	//
	//for (auto& entry : m_KeyToValue)
	//	values.emplace_back(entry.second);
	//
	//return values;
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
std::string_view EnumContainer<EnumType>::GetKey(const Argument& value) const META_NOEXCEPT
{
	typedef typename std::underlying_type< EnumType >::type UnderlyingType;

	auto type = value.GetType();

	// invalid type
	if (type != TypeOf(EnumType) &&
		type != TypeOf(UnderlyingType))
	{
		return NullObject::NullString;
	}

	auto& converted = value.GetValue<EnumType>();

	for (auto& entry : m_KeyToValue)
	{
		if (entry.second == converted)
			return entry.first;
	}

	return std::string_view();
}

///////////////////////////////////////////////////////////////////////

template<typename EnumType>
Variant EnumContainer<EnumType>::GetValue(const std::string& key) const META_NOEXCEPT
{
	auto search = m_KeyToValue.find(key);

	// not found
	if (search == m_KeyToValue.end())
		return NullObject::NullVariant;

	return Variant(search->second);
}

template<typename EnumType>
const AttributeManager& EnumContainer<EnumType>::GetNameAttributes(std::string_view name) const
{
	auto search = m_KeyToMeta.find(name);

	// not found
	if (search == m_KeyToMeta.end())
		return AttributeManager::MetaManagerNull;

	return search->second;
}

template <typename EnumType>
bool EnumContainer<EnumType>::IsValid() const META_NOEXCEPT
{
	return true;
}

END_META_NAMESPACE