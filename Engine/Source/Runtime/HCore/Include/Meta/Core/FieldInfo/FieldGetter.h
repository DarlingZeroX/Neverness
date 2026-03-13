/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** FieldGetter.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "FieldGetterBase.h"

BEGIN_META_NAMESPACE
template<typename ClassType, typename FieldType, bool IsMethod>
class FieldGetter : public FieldGetterBase
{
public:
	Variant GetValue(const Variant& obj) override;
	Variant GetValueReference(const Variant& obj) override;
};
END_META_NAMESPACE

#include "../../Impl/FieldGetter.hpp"