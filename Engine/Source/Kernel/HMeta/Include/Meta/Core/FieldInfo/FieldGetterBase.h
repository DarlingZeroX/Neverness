/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** FieldGetterBase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE
class Variant;

class FieldGetterBase
{
public:
	virtual ~FieldGetterBase(void) { }

	virtual Variant GetValue(const Variant& obj) = 0;
	virtual Variant GetValueReference(const Variant& obj) = 0;
};
END_META_NAMESPACE