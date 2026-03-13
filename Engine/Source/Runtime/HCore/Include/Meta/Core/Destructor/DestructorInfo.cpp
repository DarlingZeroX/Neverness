/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** DestructorInfo.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "DestructorInfo.h"

#include "../Variant/Variant.h"

#include "../../Deps/Logging.h"

BEGIN_META_NAMESPACE
namespace
{
	const auto kDestructorName = "destructor";
}

DestructorInfo::DestructorInfo(void)
	: Invokable()
	, m_classType(Type::Invalid()) { }

DestructorInfo::DestructorInfo(Type classType, DestructorInvokerBase* invoker)
	: Invokable(kDestructorName)
	, m_classType(classType)
	, m_invoker(invoker) { }

Type DestructorInfo::GetClassType(void) const META_NOEXCEPT
{
	return m_classType;
}

bool DestructorInfo::Invoke(Variant& obj) const
{
	const bool isValid = IsValid();
	const bool isTypeValid = m_classType == obj.GetType();

	UAssert(IsValid(), "Invalid constructor invoked.");

	UAssert(m_classType == obj.GetType(), "DestructorInfo called on incompatible type.");

	if (!isValid || !isTypeValid)
		return false;

	m_invoker->Invoke(obj);

	delete obj.m_pContainer;

	obj.m_pContainer = nullptr;

	return true;
}

bool DestructorInfo::operator==(const DestructorInfo& rhs) const noexcept
{
	return m_invoker == rhs.m_invoker;
}

bool DestructorInfo::operator!=(const DestructorInfo& rhs) const noexcept
{
	return !((*this) == rhs);
}

const DestructorInfo& DestructorInfo::Invalid(void) META_NOEXCEPT
{
	static DestructorInfo invalid;

	return invalid;
}

bool DestructorInfo::IsValid(void) const META_NOEXCEPT
{
	return m_invoker != nullptr;
}
END_META_NAMESPACE