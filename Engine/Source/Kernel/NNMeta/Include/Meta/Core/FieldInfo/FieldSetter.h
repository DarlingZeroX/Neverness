/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** FieldSetter.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "FieldSetterBase.h"

BEGIN_META_NAMESPACE
template<typename ClassType, typename FieldType, bool IsMethod>
class FieldSetter : public FieldSetterBase
{
public:
	void SetValue(Variant& obj, const Variant& value) override;
};
END_META_NAMESPACE

#include "../../Impl/FieldSetter.hpp"