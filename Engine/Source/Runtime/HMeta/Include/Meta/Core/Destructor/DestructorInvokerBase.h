/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** DestructorInvokerBase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE
class Variant;

class DestructorInvokerBase
{
public:
	virtual ~DestructorInvokerBase(void) = default;

	virtual void Invoke(const Variant& obj) = 0;
};

END_META_NAMESPACE