/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ConstructorInfo.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "ConstructorInfo.h"
#include "../Invokable.h"
#include "../../Type.h"

#include "../../Deps/Logging.h"

BEGIN_META_NAMESPACE
namespace
{
	const auto kConstructorName = "constructor";
}

ConstructorInfo::ConstructorInfo(void)
	: Invokable()
	, m_isDynamic(false)
	, m_classType(InvalidTypeID)
	, m_invoker(nullptr) { }

ConstructorInfo::ConstructorInfo(const ConstructorInfo& rhs)
	: Invokable(kConstructorName)
	, m_isDynamic(rhs.m_isDynamic)
	, m_classType(rhs.m_classType)
	, m_invoker(rhs.m_invoker)
{
	m_signature = rhs.m_signature;
}

ConstructorInfo::ConstructorInfo(const ConstructorInfo&& rhs) META_NOEXCEPT
	: Invokable(kConstructorName)
	, m_isDynamic(rhs.m_isDynamic)
	, m_classType(rhs.m_classType)
	, m_invoker(std::move(rhs.m_invoker))
{
	m_signature = std::move(rhs.m_signature);
}

ConstructorInfo::ConstructorInfo(
	Type classType,
	InvokableSignature signature,
	ConstructorInvokerBase* invoker,
	bool isDynamic
)
	: Invokable(kConstructorName)
	, m_isDynamic(isDynamic)
	, m_classType(classType)
	, m_invoker(invoker)
{
	m_signature = signature;
}

ConstructorInfo& ConstructorInfo::operator=(const ConstructorInfo& rhs)
{
	m_isDynamic = rhs.m_isDynamic;
	m_classType = rhs.m_classType;
	m_invoker = rhs.m_invoker;

	m_signature = rhs.m_signature;

	return *this;
}

ConstructorInfo& ConstructorInfo::operator=(const ConstructorInfo&& rhs)
{
	m_isDynamic = rhs.m_isDynamic;
	m_classType = rhs.m_classType;
	m_invoker = std::move(rhs.m_invoker);

	m_signature = std::move(rhs.m_signature);

	return *this;
}

const ConstructorInfo& ConstructorInfo::Invalid(void)
{
	static ConstructorInfo invalid;

	return invalid;
}

Type ConstructorInfo::GetClassType(void) const
{
	return m_classType;
}

bool ConstructorInfo::IsValid(void) const
{
	return m_invoker != nullptr;
}

bool ConstructorInfo::IsDynamic(void) const
{
	return m_isDynamic;
}

Variant ConstructorInfo::InvokeVariadic(const ArgumentList& arguments) const
{
	UAssert(IsValid(), "Invalid constructor invoked.");

	return m_invoker->Invoke(arguments);
}

bool ConstructorInfo::operator==(const ConstructorInfo& rhs) const noexcept
{
	return m_invoker == rhs.m_invoker;
}

bool ConstructorInfo::operator!=(const ConstructorInfo& rhs) const noexcept
{
	return !(*this == rhs);
}

END_META_NAMESPACE