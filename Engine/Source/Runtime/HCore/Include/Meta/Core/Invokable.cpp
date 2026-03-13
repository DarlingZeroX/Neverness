/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Invokable.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "Invokable.h"

BEGIN_META_NAMESPACE
Invokable::Invokable(const std::string& name)
	: m_name(name) { }

const InvokableSignature& Invokable::GetSignature(void) const
{
	return m_signature;
}

const std::string& Invokable::GetName(void) const
{
	return m_name;
}
END_META_NAMESPACE