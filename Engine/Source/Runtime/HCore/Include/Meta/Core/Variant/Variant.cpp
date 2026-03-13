/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Variant.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "Variant.h"

#include "../../Type.h"

BEGIN_META_NAMESPACE

Variant::Variant(void) META_NOEXCEPT
	: m_isConst(true)
	, m_isValid(false)
	, m_pContainer(NullVariantBase::Instance())
{ }

Variant::Variant(const Variant& rhs)
	: m_isConst(rhs.m_isConst)
	, m_isValid(rhs.m_isValid)
	, m_pContainer(rhs.m_pContainer ? rhs.m_pContainer->Clone() : nullptr)
{ }

Variant::Variant(Variant& rhs)
	: m_isConst(rhs.m_isConst)
	, m_isValid(rhs.m_isValid)
	, m_pContainer(rhs.m_pContainer ? rhs.m_pContainer->Clone() : nullptr)
{ }

Variant::Variant(Variant&& rhs)
	: m_isConst(rhs.m_isConst)
	, m_isValid(rhs.m_isValid)
	, m_pContainer(rhs.m_pContainer)
{
	rhs.m_isConst = true;
	rhs.m_pContainer = NullVariantBase::Instance();
}

Variant::~Variant(void)
{
	if (m_isValid)
		delete m_pContainer;
}

Variant& Variant::operator=(const Variant& rhs)
{
	Variant(rhs).Swap(*this);

	return *this;
}

Variant& Variant::operator=(Variant&& rhs)
{
	rhs.Swap(*this);

	Variant().Swap(rhs);

	return *this;
}

Variant::operator bool(void) const
{
	return ToBool();
}

Variant::operator char() const
{
	return *static_cast<char*>(getPtr());
}

Variant::operator int() const
{
	return ToInt();
}

Variant::operator unsigned() const
{
	return *static_cast<unsigned*>(getPtr());
}

Variant::operator float() const
{
	return ToFloat();
}

Variant::operator double() const
{
	return ToDouble();
}

Variant::operator std::string() const
{
	return ToString();
}

Type Variant::GetType(void) const
{
	return m_pContainer->GetType();
}

ArrayWrapper Variant::GetArray(void) const
{
	return m_pContainer->GetArray();
}

int Variant::ToInt(void) const
{
	return m_pContainer->ToInt();
}

long Variant::ToLong() const
{
	return m_pContainer->ToLong();
}

bool Variant::ToBool(void) const
{
	return m_pContainer->ToBool();
}

float Variant::ToFloat(void) const
{
	return m_pContainer->ToFloat();
}

double Variant::ToDouble(void) const
{
	return m_pContainer->ToDouble();
}

std::string Variant::ToString(void) const
{
	return m_pContainer->ToString();
}

Json Variant::SerializeJson(void) const
{
	return GetType().SerializeJson(*this);
}

void Variant::Swap(Variant& other)
{
	std::swap(m_pContainer, other.m_pContainer);
}

std::int8_t Variant::ToInt8() const
{
	return m_pContainer->ToInt8();
}

std::int16_t Variant::ToInt16() const
{
	return m_pContainer->ToInt16();
}

std::int32_t Variant::ToInt32() const
{
	return m_pContainer->ToInt32();
}

std::int64_t Variant::ToInt64() const
{
	return m_pContainer->ToInt64();
}

std::uint8_t Variant::ToUint8() const
{
	return m_pContainer->ToUint8();
}

std::uint16_t Variant::ToUint16() const
{
	return m_pContainer->ToUint16();
}

std::uint32_t Variant::ToUint32() const
{
	return m_pContainer->ToUint32();
}

std::uint64_t Variant::ToUint64() const
{
	return m_pContainer->ToUint64();
}

bool Variant::IsValid(void) const
{
	return m_isValid;
}

bool Variant::IsConst(void) const
{
	return m_isConst;
}

bool Variant::IsArray(void) const
{
	return m_pContainer->IsArray();
}

void* Variant::getPtr(void) const
{
	return m_pContainer->GetPtr();
}

END_META_NAMESPACE