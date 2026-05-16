/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** FieldSetterBase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE
class Variant;

class FieldSetterBase
{
public:
	virtual ~FieldSetterBase(void) { }

	virtual void SetValue(Variant& obj, const Variant& value) {};

	virtual void SetValue(const Argument& value) {}
};
END_META_NAMESPACE