/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** AttributeManager.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
#include "../Attribute.h"

#include "../Deps/JsonConfig.h"

#include <string>
#include <map>
#include <vector>

BEGIN_META_NAMESPACE
class Type;
class Variant;

class HMETA_API AttributeManager
{
public:
	typedef std::initializer_list<
		std::pair<Type, const Attribute*>
	> Initializer;

	static AttributeManager MetaManagerNull;

	AttributeManager(void);
	AttributeManager(const AttributeManager& rhs);
	AttributeManager(const AttributeManager&& rhs);

	const AttributeManager& operator=(const AttributeManager& rhs);

	AttributeManager(const Initializer& properties);

	~AttributeManager(void);

public:
	Variant GetAttribute(Type type) const;

	template<typename PropertyType>
	const PropertyType* GetAttribute(void) const;

	// Sets the given property
	void SetAttribute(Type type, const Attribute* value);

	// Gets all properties
	std::vector<Variant> GetAttributes(void) const;

	/** @brief Determines if this type is valid.
	*  @return true if the internal id is not Type::INVALID
	*/
	bool IsValid(void) const;

	Json SerializeJson(void) const;

private:
	void copy(const AttributeManager& rhs);

	std::map<Type, const Attribute*> m_properties;

	bool m_isValid;
};
END_META_NAMESPACE

#include "../Impl/AttributeManager.hpp"
