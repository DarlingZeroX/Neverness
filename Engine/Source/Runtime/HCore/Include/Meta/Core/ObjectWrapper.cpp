/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ObjectWrapper.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "ObjectWrapper.h"
#include "Object.h"

#include "../Type.h"

BEGIN_META_NAMESPACE
ObjectWrapper::ObjectWrapper(Object* instance)
	: m_object(instance)
{
}

Type ObjectWrapper::GetType(void) const
{
	return m_object->GetType();
}

void* ObjectWrapper::GetPtr(void) const
{
	return m_object;
}

int ObjectWrapper::ToInt(void) const
{
	return int();
}

bool ObjectWrapper::ToBool(void) const
{
	return bool();
}

float ObjectWrapper::ToFloat(void) const
{
	return float();
}

double ObjectWrapper::ToDouble(void) const
{
	return double();
}

std::string ObjectWrapper::ToString(void) const
{
	return std::string();
}

std::int8_t ObjectWrapper::ToInt8() const
{
	return std::int8_t();
}

std::int16_t ObjectWrapper::ToInt16() const
{
	return std::int16_t();
}

std::int32_t ObjectWrapper::ToInt32() const
{
	return std::int32_t();
}

std::int64_t ObjectWrapper::ToInt64() const
{
	return std::int64_t();
}

std::uint8_t ObjectWrapper::ToUint8() const
{
	return std::uint8_t();
}

std::uint16_t ObjectWrapper::ToUint16() const
{
	return std::uint16_t();
}

std::uint32_t ObjectWrapper::ToUint32() const
{
	return std::uint32_t();
}
std::uint64_t ObjectWrapper::ToUint64() const
{
	return std::uint64_t();
}

long ObjectWrapper::ToLong() const
{
	return long();
}

VariantBase* ObjectWrapper::Clone(void) const
{
	return new ObjectWrapper(m_object);
}

bool ObjectWrapper::IsValid() const
{
	return true;
}

//void ObjectWrapper::OnSerialize(Json::object& output) const
//{
//	m_object->OnSerialize(output);
//}
//
//void ObjectWrapper::OnDeserialize(const Json& input)
//{
//	m_object->OnDeserialize(input);
//}
END_META_NAMESPACE