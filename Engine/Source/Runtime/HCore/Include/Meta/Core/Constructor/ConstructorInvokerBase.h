/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ConstructorInvokerBase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../InvokableConfig.h"
#include "../Argument/ArgumentConfig.h"

BEGIN_META_NAMESPACE
class Variant;
class Argument;

class ConstructorInvokerBase
{
public:
	virtual ~ConstructorInvokerBase(void) = default;

	virtual Variant Invoke(const ArgumentList& arguments) = 0;
};
END_META_NAMESPACE