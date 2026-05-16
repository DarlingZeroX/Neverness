/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** DestructorInvoker.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "DestructorInvokerBase.h"

BEGIN_META_NAMESPACE
template<typename ClassType>
class DestructorInvoker : public DestructorInvokerBase
{
public:
	~DestructorInvoker(void) override = default;

	void Invoke(const Variant& obj) override
	{
		ClassType& instance = obj.GetValue<ClassType>();

		instance.~ClassType();
	}
};
END_META_NAMESPACE