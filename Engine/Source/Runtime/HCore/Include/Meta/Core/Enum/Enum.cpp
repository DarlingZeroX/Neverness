#include "pch.h"

#include "Enum.h"

#include "../NullObject.h"

#include "../AttributeManager.h"
#include "../Variant/Variant.h"

BEGIN_META_NAMESPACE

Enum::Enum(const EnumBase* base) META_NOEXCEPT
	: m_pContainer(base)
{
	if (m_pContainer == nullptr)
		m_pContainer = std::make_shared<NullEnumBase>();
}

Enum::Iterator::Iterator(const Enum& type, unsigned index)
	:m_enum(&type)
	, m_index(index)
{
	if (m_index == m_enum->GetNames().size())
		m_value = value_type();
	else
		m_value = { m_enum->GetNames()[m_index],m_enum->GetValues()[m_index] };
}

Enum::Iterator& Enum::Iterator::operator=(const Iterator& src)
{
	m_enum = src.m_enum;
	m_index = src.m_index;
	m_value = src.m_value;

	return *this;
}

const Enum::Iterator::value_type* Enum::Iterator::operator->() const
{
	return &m_value;
}

const Enum::Iterator::value_type& Enum::Iterator::operator*() const
{
	return m_value;
}

Enum::Iterator& Enum::Iterator::operator++()
{
	++m_index;

	if (m_index == m_enum->GetNames().size())
	{
		m_value = value_type();
	}
	else
	{
		m_value = { m_enum->GetNames()[m_index],m_enum->GetValues()[m_index] };
	}

	return *this;
}

Enum::Iterator Enum::Iterator::operator++(int)
{
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

const Enum::Iterator Enum::begin() const
{
	return Iterator(*this, 0);
}

const Enum::Iterator Enum::end() const
{
	return Iterator(*this, GetNames().size());
}

bool Enum::IsValid(void) const META_NOEXCEPT
{
	return m_pContainer->IsValid();
}

bool Enum::operator==(const Enum& rhs) const META_NOEXCEPT
{
	return m_pContainer == rhs.m_pContainer;
}

bool Enum::operator!=(const Enum& rhs) const META_NOEXCEPT
{
	return m_pContainer != rhs.m_pContainer;
}

const std::string_view& Enum::GetName(void) const META_NOEXCEPT
{
	return m_pContainer->GetName();
}

Type Enum::GetType(void) const
{
	return m_pContainer->GetType();
}

Type Enum::GetParentType(void) const
{
	return m_pContainer->GetParentType();
}

Type Enum::GetUnderlyingType(void) const
{
	return m_pContainer->GetUnderlyingType();
}

const std::vector<std::string_view>& Enum::GetNames(void) const META_NOEXCEPT
{
	return m_pContainer->GetKeys();
}

const std::vector<Variant>& Enum::GetValues(void) const
{
	return m_pContainer->GetValues();
}

std::string_view Enum::GetName(const Argument& value) const
{
	return m_pContainer->GetKey(value);
}

Variant Enum::GetValue(const std::string& key) const
{
	return m_pContainer->GetValue(key);
}

const AttributeManager& Enum::GetNameAttributes(const std::string& key) const
{
	return m_pContainer->GetNameAttributes(key);
}

const Variant& Enum::operator[](const std::string& name) const
{
	return GetValue(name);
}

const std::string_view& Enum::operator()(const Argument& value) const
{
	return GetName(value);
}

END_META_NAMESPACE