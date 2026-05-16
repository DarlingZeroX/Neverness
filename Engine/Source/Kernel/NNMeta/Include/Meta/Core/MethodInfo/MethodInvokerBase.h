/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** MethodInvokerBase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../InvokableConfig.h"
#include "../Argument/ArgumentConfig.h"
#include "../Variant/Variant.h"

BEGIN_META_NAMESPACE
class Variant;

class MethodInvokerBase
{
public:
	virtual ~MethodInvokerBase(void) { }

	virtual Variant Invoke(Variant& obj, const ArgumentList& arguments) = 0;
	virtual Type GetReturnType() { return Type(); }
};
END_META_NAMESPACE