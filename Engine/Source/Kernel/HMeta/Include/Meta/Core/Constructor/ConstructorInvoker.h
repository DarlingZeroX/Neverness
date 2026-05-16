/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ConstructorInvoker.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "ConstructorInvokerBase.h"

BEGIN_META_NAMESPACE
class Variant;
class Argument;

template<typename ClassType, bool IsDynamic, bool IsObjectWrapped, typename ...ArgTypes>
class ConstructorInvoker : public ConstructorInvokerBase
{
public:
	static_assert(THIS_ARG_COUNT <= MaxArgumentCount,
		"Constructor has too many arguments. It's time to generate more overloads."
		);

	Variant Invoke(const ArgumentList& arguments) override;
};
END_META_NAMESPACE

#include "../../Impl/ConstructorInvoker.hpp"