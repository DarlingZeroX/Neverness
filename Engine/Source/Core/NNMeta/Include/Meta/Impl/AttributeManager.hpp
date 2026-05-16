/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** AttributeManager.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Type.h"

BEGIN_META_NAMESPACE
template<typename PropertyType>
const PropertyType* AttributeManager::GetAttribute(void) const
{
	static_assert(std::is_base_of<Attribute, PropertyType>::value,
		"Type must be a Attribute."
		);

	static const auto type = TypeOf(PropertyType);

	auto search = m_properties.find(type);

	// doesn't exist
	if (search == m_properties.end())
		return nullptr;

	return static_cast<const PropertyType*>(search->second);
}
END_META_NAMESPACE