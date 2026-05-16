/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Attribute.hpp
** --------------------------------------------------------------------------*/

#pragma once
#include <utility>

BEGIN_META_NAMESPACE
template<typename AttributeType, typename ...Args>
Attribute* MetaPropertyInitializer(Args&&... args)
{
	static_assert(std::is_base_of<Attribute, AttributeType>::value, "Meta properties must inherit from Attribute");

	return new AttributeType(std::forward<Args>(args)...);
}
END_META_NAMESPACE