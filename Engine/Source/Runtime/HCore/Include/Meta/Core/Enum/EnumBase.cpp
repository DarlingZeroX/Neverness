#include "pch.h"

#include "EnumBase.h"
#include "../NullObject.h"

#include "../Variant/Variant.h"
#include "../Argument/Argument.h"
#include "Meta/Core/AttributeManager.h"

BEGIN_META_NAMESPACE
EnumBase::EnumBase(std::string_view name, TypeID owner)
	: m_parentType(owner)
	, m_name(name) { }

Type EnumBase::GetParentType(void) const
{
	return m_parentType;
}

std::string_view EnumBase::GetName(void) const META_NOEXCEPT
{
	return m_name;
}

NullEnumBase::NullEnumBase()
	: EnumBase("", 0)
{
}

Type NullEnumBase::GetType() const noexcept
{
	return Type::Invalid();
}

Type NullEnumBase::GetUnderlyingType() const noexcept
{
	return Type::Invalid();
}

const std::vector<std::string_view>& NullEnumBase::GetKeys() const noexcept
{
	return NullObject::NullStringViewVector;
}

const std::vector<Variant>& NullEnumBase::GetValues() const noexcept
{
	return NullObject::NullVariantVector;
}

std::string_view NullEnumBase::GetKey(const Argument& value) const noexcept
{
	return NullObject::NullString;
}

Variant NullEnumBase::GetValue(const std::string& key) const noexcept
{
	return NullObject::NullVariant;
}

const AttributeManager& NullEnumBase::GetNameAttributes(std::string_view name) const
{
	return AttributeManager::MetaManagerNull;
}

bool NullEnumBase::IsValid() const META_NOEXCEPT
{
	return false;
}

std::shared_ptr<NullEnumBase> NullEnumBase::Instance()
{
	static std::shared_ptr<NullEnumBase> nullInstance = std::make_shared<NullEnumBase>();

	return nullInstance;
}

END_META_NAMESPACE