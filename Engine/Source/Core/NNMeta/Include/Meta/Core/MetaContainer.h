/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** MetaContainer.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
#include "AttributeManager.h"

BEGIN_META_NAMESPACE
class HMETA_API MetaContainer
{
public:
	const AttributeManager& GetMeta(void) const;

	MetaContainer() = default;
	MetaContainer(const MetaContainer& rhs) = default;
	MetaContainer& operator=(const MetaContainer& rhs) = default;
private:
	//MetaContainer& operator=(const MetaContainer& rhs) = delete;

	friend class ReflectionDatabase;
	friend struct TypeData;

	AttributeManager m_meta;
};
END_META_NAMESPACE