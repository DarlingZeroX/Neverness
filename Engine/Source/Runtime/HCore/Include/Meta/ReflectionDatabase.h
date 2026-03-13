/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ReflectionDatabase.h
** --------------------------------------------------------------------------*/

#pragma once
#include "MetaCommonDefinitions.h"
#include "Core/Type/TypeData.h"
#include "Core/Type/TypeInfo.h"

#include <vector>
#include <unordered_map>
#include "../Core/HTypeInfo.h"

BEGIN_META_NAMESPACE

struct TInfo
{
	TypeID ID = InvalidTypeID;
	bool Defined = false;
};

class HMETA_API ReflectionDatabase
{
public:
	ReflectionDatabase(void);
	~ReflectionDatabase(void);

	Horizon::unordered_type_map<TInfo> typeInfos;

	std::vector<TypeData> types;

	std::unordered_map<std::string, TypeID> ids;

	std::unordered_map<std::string, FieldInfo> globals;

	std::unordered_map<std::string, InvokableOverloadMap<MethodInfo>> globalFunctions;

	//template<typename T>
	//TypeData& GetTypeData();

	TypeID GetTypeID(const type_info* info);
	bool TypeDefined(const type_info* info);
	void TypeSetDefined(const type_info* info);

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	static ReflectionDatabase& Instance(void);

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	TypeID RegisterType(const type_info* typeInfo, const std::string& fullName);

private:
	TypeID AllocateType(const std::string& fullName);

	void Initialize();
public:

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	// Function Getter, Function Setter
	template<typename GlobalType, typename GetterType, typename SetterType>
	void AddGlobal(
		const std::string& name,
		GetterType getter,
		SetterType setter,
		const AttributeManager::Initializer& meta
	);

	// Function Getter, Raw Setter
	template<typename GlobalType, typename GetterType>
	void AddGlobal(
		const std::string& name,
		GetterType getter,
		GlobalType* globalSetter,
		const AttributeManager::Initializer& meta
	);

	// Raw Getter, Function Setter
	template<typename GlobalType, typename SetterType>
	void AddGlobal(
		const std::string& name,
		GlobalType* globalGetter,
		SetterType setter,
		const AttributeManager::Initializer& meta
	);

	// Raw Getter, Raw Setter
	template<typename GlobalType>
	void AddGlobal(
		const std::string& name,
		GlobalType* globalGetter,
		GlobalType* globalSetter,
		const AttributeManager::Initializer& meta
	);

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	template<typename FunctionType>
	void AddGlobalFunction(
		const std::string& name,
		FunctionType globalFunction,
		const AttributeManager::Initializer& meta
	);

	const MethodInfo& GetGlobalFunction(const std::string& name);

	const MethodInfo& GetGlobalFunction(const std::string& name, const InvokableSignature& signature);

private:
	TypeID m_nextID;
};

template <typename T>
static void ReflectTypeRegister(
	ReflectionDatabase& db,
	const std::string& spaceName,
	const std::string& typeName,
	bool beingDefined = true
)
{
	std::string fullName = spaceName + "::" + typeName;

	auto id = db.RegisterType(&typeid(T),fullName);
	//auto id = db.AllocateType(fullName);
	auto& type = db.types[id];
	type.name = typeName;
	type.spaceName = spaceName;

	TypeInfo::Register<T>(id, type, beingDefined);
}

template <typename T>
static bool ReflectTypeDefinable()
{
	//auto typeID = typeidof(T);
	auto typeID = TypeIDOf(T);

	return typeID != InvalidTypeID && !TypeInfo::Defined<T>();
}

template <typename T>
static void DefineReflectType(ReflectionDatabase& db, void(*typeDefineFunc)(ReflectionDatabase&, TypeData&, TypeID&))
{
	namespace m = META_NAMESPACE;

	auto typeID = TypeIDOf(T);

	if (typeID != m::InvalidTypeID && !m::TypeInfo::Defined<T>())
	{
		auto& type = db.types[typeID];

		typeDefineFunc(db, type, typeID);

		META_NAMESPACE::TypeInfo::SetDefined<T>();
	}
}

END_META_NAMESPACE

#include "Impl/ReflectionDatabase.hpp"