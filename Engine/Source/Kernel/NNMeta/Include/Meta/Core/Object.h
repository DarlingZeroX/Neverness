/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Object.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
//#include "JsonConfig.h"

// Constructs a variant that wraps an object
#define ObjectVariant(object) META_NAMESPACE::Variant { object, META_NAMESPACE::VariantPolicy::WrapObject( ) }

BEGIN_META_NAMESPACE
class Type;

class Object
{
public:
	virtual ~Object(void) { }

	virtual Type GetType(void) const = 0;
	virtual Object* Clone(void) const = 0;

	//virtual void OnSerialize(Json::object& output) const { }
	//virtual void OnDeserialize(const Json& input) { }
};
END_META_NAMESPACE