/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeData.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "TypeData.h"

#include "../../ReflectionDatabase.h"

BEGIN_META_NAMESPACE
TypeData::TypeData(void)
	: isEnum(false)
	, isPrimitive(false)
	, isPointer(false)
	, isClass(false)
	, enumeration{ nullptr }  { }

///////////////////////////////////////////////////////////////////////

TypeData::TypeData(const std::string& name)
	: isEnum(false)
	, isPrimitive(false)
	, isPointer(false)
	, isClass(false)
	, name(name)
	, enumeration{ nullptr } { }

void TypeData::LoadBaseClasses(
	ReflectionDatabase& db,
	TypeID thisType,
	const std::initializer_list<Type>& classes
)
{
	for (auto& base : classes)
	{
		// skip invalid types
		if (base == Type::Invalid())
			continue;

		baseClasses.insert(base);

		// add this type to the base type's derived classes
		db.types[base.m_id].derivedClasses.insert(thisType);
	}
}

///////////////////////////////////////////////////////////////////////

const ConstructorInfo& TypeData::GetConstructor(
	const InvokableSignature& signature
)
{
	auto search = constructors.find(signature);

	if (search == constructors.end())
		return ConstructorInfo::Invalid();

	return search->second;
}

const ConstructorInfo& TypeData::GetDynamicConstructor(
	const InvokableSignature& signature
)
{
	auto search = dynamicConstructors.find(signature);

	if (search == dynamicConstructors.end())
		return ConstructorInfo::Invalid();

	return search->second;
}

///////////////////////////////////////////////////////////////////////

const FieldInfo& TypeData::GetField(const std::string& name) const
{
	for (auto& field : fields)
		if (field.GetName() == name)
			return field;

	return FieldInfo::Invalid();
}

///////////////////////////////////////////////////////////////////////

const FieldInfo& TypeData::GetStaticField(const std::string& name) const
{
	for (auto& field : staticFields)
		if (field.GetName() == name)
			return field;

	return FieldInfo::Invalid();
}

const MethodInfo& TypeData::GetMethod(const std::string& name)
{
	auto& base = methods[name];

	if (!base.size())
		return MethodInfo::Invalid();

	return base.begin()->second;
}

///////////////////////////////////////////////////////////////////////

const MethodInfo& TypeData::GetMethod(
	const std::string& name,
	const InvokableSignature& signature
)
{
	auto& base = methods[name];

	auto search = base.find(signature);

	if (search == base.end())
		return MethodInfo::Invalid();

	return search->second;
}

///////////////////////////////////////////////////////////////////////

const MethodInfo& TypeData::GetStaticMethod(const std::string& name)
{
	auto& base = staticMethods[name];

	if (!base.size())
		return MethodInfo::Invalid();

	return base.begin()->second;
}

///////////////////////////////////////////////////////////////////////

const MethodInfo& TypeData::GetStaticMethod(
	const std::string& name,
	const InvokableSignature& signature
)
{
	auto& base = staticMethods[name];

	auto search = base.find(signature);

	if (search == base.end())
		return MethodInfo::Invalid();

	return search->second;
}
END_META_NAMESPACE