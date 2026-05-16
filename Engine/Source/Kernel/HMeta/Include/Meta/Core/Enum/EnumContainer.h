/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** EnumContainer.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "EnumBase.h"

#include "../Variant/Variant.h"

#include <unordered_map>

BEGIN_META_NAMESPACE

template<typename EnumType>
class EnumContainer : public EnumBase
{
public:
	typedef std::initializer_list< std::tuple<
		std::string_view,
		EnumType,
		AttributeManager
		>> Initializer;
	typedef std::unordered_map<std::string_view, EnumType> Table;
	typedef std::unordered_map<std::string_view, AttributeManager> MetaTable;

	EnumContainer(const std::string& name, const Initializer& initializer);
	EnumContainer(const std::string& name, const Initializer& initializer, TypeID owner);

	Type GetType(void) const META_NOEXCEPT override;
	Type GetUnderlyingType(void) const META_NOEXCEPT override;

	const std::vector<std::string_view>& GetKeys(void) const META_NOEXCEPT override;
	const std::vector<Variant>& GetValues(void) const META_NOEXCEPT override;

	std::string_view GetKey(const Argument& value) const META_NOEXCEPT override;
	Variant GetValue(const std::string& key) const META_NOEXCEPT override;

	const AttributeManager& GetNameAttributes(std::string_view name) const override;

	bool IsValid() const META_NOEXCEPT override;
private:
	void Initialize(const std::string& name, const Initializer& initializer);

	Table m_KeyToValue;
	MetaTable m_KeyToMeta;

	std::vector<std::string_view> m_Keys;
	std::vector<Variant> m_Values;
};

END_META_NAMESPACE

#include "../../Impl/EnumContainer.hpp"