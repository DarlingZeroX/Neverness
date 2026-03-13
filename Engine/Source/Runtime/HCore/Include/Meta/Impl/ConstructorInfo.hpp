/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ConstructorInfo.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Variant/Variant.h"
#include "../Argument/Argument.h"

BEGIN_META_NAMESPACE
template<typename ...Args>
Variant ConstructorInfo::Invoke(Args &&...args) const
{
	ArgumentList arguments{ std::forward<Args>(args)... };

	return InvokeVariadic(arguments);
}
END_META_NAMESPACE