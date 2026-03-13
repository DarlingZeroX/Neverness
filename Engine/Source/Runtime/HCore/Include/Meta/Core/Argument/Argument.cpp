/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Argument.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "Argument.h"

#include "../../Type.h"

#include "../Variant/Variant.h"

BEGIN_META_NAMESPACE
Argument::Argument(void) META_NOEXCEPT
	: m_typeID(InvalidTypeID)
	, m_isArray(false)
	, m_data(nullptr) { }

Argument::Argument(const Argument& rhs)
	: m_typeID(rhs.m_typeID)
	, m_isArray(rhs.m_isArray)
	, m_data(rhs.m_data) { }

Argument::Argument(Variant& obj)
	: m_typeID(obj.GetType().GetID())
	, m_isArray(obj.GetType().IsArray())
	, m_data(obj.getPtr()) { }

Argument::Argument(const Variant& obj)
	: m_typeID(obj.GetType().GetID())
	, m_isArray(obj.GetType().IsArray())
	, m_data(obj.getPtr()) { }

Argument& Argument::operator=(const Argument& rhs)
{
	m_data = rhs.m_data;

	const_cast<TypeID&>(m_typeID) = rhs.m_typeID;

	return *this;
}

bool Argument::operator==(const Argument& rhs) const
{
	return this->m_typeID == rhs.m_typeID && this->m_data == rhs.m_data && this->m_isArray == rhs.m_isArray;
}

bool Argument::operator!=(const Argument& rhs) const
{
	return !(*this == rhs);
}

Type Argument::GetType(void) const
{
	return Type(m_typeID, m_isArray);
}

void* Argument::GetPtr(void) const META_NOEXCEPT
{
	return const_cast<void*>(m_data);
}
END_META_NAMESPACE