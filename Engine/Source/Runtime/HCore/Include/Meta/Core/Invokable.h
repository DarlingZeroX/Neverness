/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Invokable.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
#include "InvokableConfig.h"
#include "Argument/ArgumentConfig.h"

#include <vector>

BEGIN_META_NAMESPACE
class Type;

class HMETA_API Invokable
{
public:
	typedef std::vector < std::string > ParamNameList;
public:
	Invokable(const std::string& name = "INVALID");

	Invokable(const Invokable&) = default;
	Invokable& operator=(const Invokable&) = default;

	template<typename ...Types>
	static InvokableSignature CreateSignature(void);

	const InvokableSignature& GetSignature(void) const;

	const std::string& GetName(void) const;

protected:
	std::string m_name;

	InvokableSignature m_signature;
};
END_META_NAMESPACE

#include "../Impl/Invokable.hpp"