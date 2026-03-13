/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Logging.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"

#define UAssert(assertion, ...) (!!(assertion) ||                                 \
    (META_NAMESPACE::logging::Assert(__FILE__, __FUNCTION__, __LINE__,##__VA_ARGS__), 0)) \

BEGIN_META_NAMESPACE
namespace logging
{
	template<typename... Args>
	void Assert(
		const std::string& file,
		const std::string& function,
		unsigned line,
		const std::string& format,
		const Args&... args
	);
}

END_META_NAMESPACE

#include "Logging.hpp"