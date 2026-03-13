/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeCreator.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "TypeCreator.h"

BEGIN_META_NAMESPACE
Variant TypeCreator::CreateVariadic(const Type& type, const ArgumentList& arguments)
{
	InvokableSignature signature;

	for (auto& argument : arguments)
		signature.emplace_back(argument.GetType());

	auto& constructor = type.GetConstructor(signature);

	return constructor.InvokeVariadic(arguments);
}

Variant TypeCreator::CreateDynamicVariadic(const Type& type, const ArgumentList& arguments)
{
	InvokableSignature signature;

	for (auto& argument : arguments)
		signature.emplace_back(argument.GetType());

	auto& constructor = type.GetDynamicConstructor(signature);

	return constructor.InvokeVariadic(arguments);
}
END_META_NAMESPACE