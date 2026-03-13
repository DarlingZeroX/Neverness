/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** RuntimeMetaProperties.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
#include "../Attribute.h"
#include "../Type.h"

/** @brief Ensures associative enum values are serialized as their literal value.
 */
class SerializeAsNumber : public META_NAMESPACE::Attribute
{
	META_OBJECT;
};