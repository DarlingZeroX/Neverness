/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** MetaContainer.cpp
** --------------------------------------------------------------------------*/



#include "MetaContainer.h"
#include "AttributeManager.h"

BEGIN_META_NAMESPACE
const AttributeManager& MetaContainer::GetMeta(void) const
{
	return m_meta;
}
END_META_NAMESPACE