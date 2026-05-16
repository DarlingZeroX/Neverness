/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ReflectionDatabase.hpp
** --------------------------------------------------------------------------*/

#pragma once
#include "../Core/FieldInfo/FieldInfo.h"

BEGIN_META_NAMESPACE

//template<typename T>
//TypeData& ReflectionDatabase::GetTypeData()
//{
//	return types[typeidof(T)];
//}

template<typename GlobalType, typename GetterType, typename SetterType>
void ReflectionDatabase::AddGlobal(const std::string& name, GetterType getter, SetterType setter, const AttributeManager::Initializer& meta)
{
	typedef GlobalGetter<GlobalType, true> GlobalGetterType;
	typedef GlobalSetter<GlobalType, true> GlobalSetterType;

	FieldInfo::InitArgs args;
	args.isStatic = true;
	args.name = name;
	args.type = TypeOf(GlobalType);
	args.classType = Type::Invalid();
	args.getter = !getter ? nullptr : new GlobalGetterType(
		reinterpret_cast<typename GlobalGetterType::Signature>(getter)
	);
	args.setter = !setter ? nullptr : new GlobalSetterType(
		reinterpret_cast<typename GlobalSetterType::Signature>(setter)
	);

	FieldInfo global(args);

	//Global global{
	//	name,
	//	TypeOf(GlobalType),
	//	!getter ? nullptr : new GlobalGetterType(
	//		reinterpret_cast<typename GlobalGetterType::Signature>(getter)
	//	),
	//	!setter ? nullptr : new GlobalSetterType(
	//		reinterpret_cast<typename GlobalSetterType::Signature>(setter)
	//	)
	//};

	global.m_meta = meta;

	globals.emplace(name, global);
}

///////////////////////////////////////////////////////////////////////

template<typename GlobalType, typename GetterType>
void ReflectionDatabase::AddGlobal(const std::string& name, GetterType getter, GlobalType* globalSetter, const AttributeManager::Initializer& meta)
{
	typedef GlobalGetter<GlobalType, true> GlobalGetterType;
	typedef GlobalSetter<GlobalType, false> GlobalSetterType;

	FieldInfo::InitArgs args;
	args.isStatic = true;
	args.name = name;
	args.type = TypeOf(GlobalType);
	args.classType = Type::Invalid();
	args.getter = !getter ? nullptr : new GlobalGetterType(
		reinterpret_cast<typename GlobalGetterType::Signature>(getter)
	);
	args.setter = !globalSetter ? nullptr : new GlobalSetterType(globalSetter);

	FieldInfo global(args);

	//Global global{
	//	name,
	//	TypeOf(GlobalType),
	//	!getter ? nullptr : new GlobalGetterType(
	//		reinterpret_cast<typename GlobalGetterType::Signature>(getter)
	//	),
	//	!globalSetter ? nullptr : new GlobalSetterType(globalSetter)
	//};
	//
	//global.m_meta = meta;

	globals.emplace(name, global);
}

///////////////////////////////////////////////////////////////////////

template<typename GlobalType, typename SetterType>
void ReflectionDatabase::AddGlobal(const std::string& name, GlobalType* globalGetter, SetterType setter, const AttributeManager::Initializer& meta)
{
	typedef GlobalGetter<GlobalType, false> GlobalGetterType;
	typedef GlobalSetter<GlobalType, true> GlobalSetterType;

	FieldInfo::InitArgs args;
	args.isStatic = true;
	args.name = name;
	args.type = TypeOf(GlobalType);
	args.classType = Type::Invalid();
	args.getter = !globalGetter ? nullptr : new GlobalGetterType(globalGetter);
	args.setter = !setter ? nullptr : new GlobalSetterType(
		reinterpret_cast<typename GlobalSetterType::Signature>(setter)
	);

	FieldInfo global(args);

	//Global global{
	//	name,
	//	TypeOf(GlobalType),
	//	!globalGetter ? nullptr : new GlobalGetterType(globalGetter),
	//	!setter ? nullptr : new GlobalSetterType(
	//		reinterpret_cast<typename GlobalSetterType::Signature>(setter)
	//	)
	//};

	global.m_meta = meta;

	globals.emplace(name, global);
}

///////////////////////////////////////////////////////////////////////

template<typename GlobalType>
void ReflectionDatabase::AddGlobal(const std::string& name, GlobalType* globalGetter, GlobalType* globalSetter, const AttributeManager::Initializer& meta)
{
	typedef GlobalGetter<GlobalType, false> GlobalGetterType;
	typedef GlobalSetter<GlobalType, false> GlobalSetterType;

	FieldInfo::InitArgs args;
	args.isStatic = true;
	args.name = name;
	args.type = TypeOf(GlobalType);
	args.classType = Type::Invalid();
	args.getter = !globalGetter ? nullptr : new GlobalGetterType(globalGetter);
	args.setter = !globalSetter ? nullptr : new GlobalSetterType(globalSetter);

	FieldInfo global(args);

	//Global global{
	//	name,
	//	TypeOf(GlobalType),
	//	!globalGetter ? nullptr : new GlobalGetterType(globalGetter),
	//	!globalSetter ? nullptr : new GlobalSetterType(globalSetter)
	//};

	global.m_meta = meta;

	globals.emplace(name, global);
}

///////////////////////////////////////////////////////////////////////

template<typename FunctionType>
void ReflectionDatabase::AddGlobalFunction(
	const std::string& name,
	FunctionType globalFunction,
	const AttributeManager::Initializer& meta
)
{
	MethodInfo function(name, globalFunction);

	function.m_meta = meta;

	globalFunctions[name].emplace(
		function.GetSignature(),
		function
	);
}
END_META_NAMESPACE