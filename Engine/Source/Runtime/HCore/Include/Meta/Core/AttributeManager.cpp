/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** AttributeManager.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "AttributeManager.h"

#include "../Type.h"
#include "Variant/Variant.h"

#include "../Deps/Logging.h"

#include <algorithm>

BEGIN_META_NAMESPACE

AttributeManager AttributeManager::MetaManagerNull;

AttributeManager::AttributeManager(void) : m_isValid(false) { }

AttributeManager::AttributeManager(const AttributeManager& rhs)
	: m_isValid(true)
{
	copy(rhs);
}

AttributeManager::AttributeManager(const AttributeManager&& rhs)
	:
	m_isValid(true),
	m_properties(std::move(rhs.m_properties))
{
}

const AttributeManager& AttributeManager::operator=(const AttributeManager& rhs)
{
	copy(rhs);
	m_isValid = true;

	return *this;
}

///////////////////////////////////////////////////////////////////////

AttributeManager::AttributeManager(const Initializer& properties)
	: m_isValid(true)
{
	for (auto& prop : properties)
	{
		UAssert(prop.first.IsValid(),
			"Invalid meta property registered."
			"Make sure the property type is enabled in reflection."
		);

		SetAttribute(prop.first, prop.second);
	}
}

AttributeManager::~AttributeManager(void)
{
	for (auto& prop : m_properties)
		delete prop.second;

	m_isValid = false;
}

///////////////////////////////////////////////////////////////////////

Variant AttributeManager::GetAttribute(Type type) const
{
	auto search = m_properties.find(type);

	if (search == m_properties.end())
		return { };

	return ObjectVariant(search->second);
}

///////////////////////////////////////////////////////////////////////

void AttributeManager::SetAttribute(Type type, const Attribute* prop)
{
	auto search = m_properties.find(type);

	// delete old property if it exists
	if (search != m_properties.end())
		delete search->second;

	m_properties[type] = prop;
}

std::vector<Variant> AttributeManager::GetAttributes(void) const
{
	std::vector<Variant> properties;

	for (auto& property : m_properties)
		properties.emplace_back(ObjectVariant(property.second));

	return properties;
}

bool AttributeManager::IsValid() const
{
	return m_isValid;
}

Json AttributeManager::SerializeJson(void) const
{
	Json::object object{ };

	for (auto& property : m_properties)
	{
		auto instance = ObjectVariant(property.second);

		auto type = instance.GetType();

		object[type.GetName().data()] = type.SerializeJson(instance);
	}

	return object;
}

void AttributeManager::copy(const AttributeManager& rhs)
{
	for (auto& prop : rhs.m_properties)
	{
		m_properties[prop.first] =
			static_cast<Attribute*>(prop.second->Clone());
	}
}
END_META_NAMESPACE