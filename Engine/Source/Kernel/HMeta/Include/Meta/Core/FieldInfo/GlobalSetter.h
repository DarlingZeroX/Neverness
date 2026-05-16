/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** GlobalSetter.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "FieldSetterBase.h"

BEGIN_META_NAMESPACE
template<typename GlobalType, bool IsMethod>
class GlobalSetter : public FieldSetterBase
{
public:
	void SetValue(const Argument& value) override;
};

END_META_NAMESPACE

#include "../../Impl/GlobalSetter.hpp"