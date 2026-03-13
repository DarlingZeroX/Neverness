/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** FieldInfo.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "FieldInfo.h"
#include "../MethodInfo/MethodInfo.h"

BEGIN_META_NAMESPACE
FieldInfo::FieldInfo(void)
	: m_isStatic(false)
	, m_type(Type::Invalid())
	, m_classType(Type::Invalid())
	, m_name("INVALID")
	, m_getter(nullptr)
	, m_setter(nullptr) { }

FieldInfo::FieldInfo(
	const std::string& name,
	Type type,
	Type classType,
	FieldGetterBase* getter,
	FieldSetterBase* setter
)
	: m_isStatic(false)
	, m_type(type)
	, m_classType(classType)
	, m_name(name)
	, m_getter(getter)
	, m_setter(setter) { }

FieldInfo::FieldInfo(const InitArgs& args)
	: m_isStatic(args.isStatic)
	, m_type(args.type)
	, m_classType(args.classType)
	, m_name(args.name)
	, m_getter(args.getter)
	, m_setter(args.setter) { }

bool FieldInfo::operator==(const FieldInfo& right) const
{
	return  this->m_type == right.m_type;
}

bool FieldInfo::operator!=(const FieldInfo& right) const
{
	return !(*this == right);
}

const FieldInfo& FieldInfo::Invalid(void)
{
	static FieldInfo field;

	return field;
}

bool FieldInfo::IsPublic() const
{
	// TODO
	return false;
}

bool FieldInfo::IsPrivate() const
{
	// TODO
	return false;
}

bool FieldInfo::IsGlobal() const
{
	return m_classType == Type::Invalid();
}

bool FieldInfo::SetValue(Variant& instance, const Variant& value, const MethodInfo& setter)
{
	// read only?
	if (!instance.IsConst())
	{
		setter.Invoke(instance, value);

		return true;
	}

	return false;
}

bool FieldInfo::IsStatic(void) const
{
	return m_isStatic;
}

bool FieldInfo::IsValid(void) const
{
	return m_getter != nullptr;
}

bool FieldInfo::IsReadOnly(void) const
{
	return m_setter == nullptr;
}

Type FieldInfo::GetType(void) const
{
	return m_type;
}

Type FieldInfo::GetClassType(void) const
{
	return m_classType;
}

const std::string& FieldInfo::GetName(void) const
{
	return m_name;
}

Variant FieldInfo::GetValue(const Variant& instance) const
{
	return m_getter->GetValue(instance);
}

Variant FieldInfo::GetValueRef(const Variant& instance) const
{
	return m_getter->GetValueReference(instance) ;
}

bool FieldInfo::SetVariantValue(Variant& instance, const Variant& value) const
{
	// read only?
	if (m_setter && !instance.IsConst())
	{
		m_setter->SetValue(instance, value);

		return true;
	}

	return false;
}

bool FieldInfo::SetValue(const Argument& value) const
{
	assert(m_isStatic);

	Variant v;

	// read only?
	if (m_setter != nullptr)
	{
		m_setter->SetValue(value);

		return true;
	}

	return false;
}

END_META_NAMESPACE