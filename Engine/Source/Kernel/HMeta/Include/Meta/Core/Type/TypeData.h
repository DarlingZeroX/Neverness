/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeData.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../AttributeManager.h"

#include "../Constructor/ConstructorInfo.h"
#include "../Destructor/DestructorInfo.h"

#include "../FieldInfo/FieldInfo.h"
#include "../MethodInfo/MethodInfo.h"

#include "../Enum/Enum.h"
#include "../Enum/EnumContainer.h"

BEGIN_META_NAMESPACE
class ReflectionDatabase;

struct HMETA_API TypeData
{
	typedef std::unordered_map<InvokableSignature, ConstructorInfo> ConstructorTable;
	typedef std::unordered_map<std::string, InvokableOverloadMap<MethodInfo>> MethodTable;
	typedef std::unordered_map<std::string, InvokableOverloadMap<MethodInfo>> FunctionTable;

	bool isEnum : 1;
	bool isPrimitive : 1;
	bool isSigned : 1;
	bool isFloatingPoint : 1;
	bool isPointer : 1;
	bool isClass : 1;

	const type_info * typeInfo;
	//bool isBoolean : 1;

	AttributeManager meta;

	std::string spaceName;
	std::string fullName;
	std::string name;

	// enum type

	Enum enumeration;

	// class type

	Type::Set baseClasses;
	Type::Set derivedClasses;

	ConstructorInfo arrayConstructor;
	DestructorInfo destructor;
	
	ConstructorTable constructors;
	ConstructorTable dynamicConstructors;
	
	std::vector<FieldInfo> fields;
	std::vector<FieldInfo> staticFields;
	
	MethodTable methods;
	FunctionTable staticMethods;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	TypeData(void);
	TypeData(const TypeData&) = default;
	TypeData& operator=(const TypeData&) = default;
	TypeData(const std::string& name);

	void LoadBaseClasses(ReflectionDatabase& db, TypeID thisType, const std::initializer_list<Type>& classes);

	///////////////////////////////////////////////////////////////////
	/////////////////////	ConstructorInfo
	///////////////////////////////////////////////////////////////////
	template<typename ClassType, bool IsDynamic, bool IsWrapped, typename ...Args>
	void AddConstructor(const AttributeManager::Initializer& meta);

	template<typename ClassType>
	void SetArrayConstructor(void);

	const ConstructorInfo& GetConstructor(const InvokableSignature& signature);
	const ConstructorInfo& GetDynamicConstructor(const InvokableSignature& signature);

	///////////////////////////////////////////////////////////////////
	/////////////////////	DestructorInfo
	///////////////////////////////////////////////////////////////////
	template<typename ClassType>
	void SetDestructor(void);

	///////////////////////////////////////////////////////////////////
	/////////////////////	Field
	///////////////////////////////////////////////////////////////////
	// Method Getter, Method Setter
	template<typename ClassType, typename FieldType, typename GetterReturnType, typename SetterArgumentType>
	void AddField(
		const std::string& name,
		GetterReturnType(ClassType::* methodGetter)(void),
		void (ClassType::* methodSetter)(SetterArgumentType),
		const AttributeManager::Initializer& meta
	);

	// Const Method Getter, Method Setter
	template<typename ClassType, typename FieldType, typename GetterReturnType, typename SetterArgumentType>
	void AddField(
		const std::string& name,
		GetterReturnType(ClassType::* methodGetter)(void) const,
		void (ClassType::* methodSetter)(SetterArgumentType),
		const AttributeManager::Initializer& meta
	);

	// Method Getter, FieldInfo Setter
	template<typename ClassType, typename FieldType, typename GetterReturnType>
	void AddField(
		const std::string& name,
		GetterReturnType(ClassType::* methodGetter)(void),
		typename FieldSetter<ClassType, FieldType, false>::Signature fieldSetter,
		const AttributeManager::Initializer& meta
	);

	// Const Method Getter, FieldInfo Setter
	template<typename ClassType, typename FieldType, typename GetterReturnType>
	void AddField(
		const std::string& name,
		GetterReturnType(ClassType::* methodGetter)(void) const,
		typename FieldSetter<ClassType, FieldType, false>::Signature fieldSetter,
		const AttributeManager::Initializer& meta
	);

	// FieldInfo Getter, Method Setter
	template<typename ClassType, typename FieldType, typename SetterArgumentType>
	void AddField(
		const std::string& name,
		typename FieldGetter<ClassType, FieldType, false>::Signature fieldGetter,
		void (ClassType::* methodSetter)(SetterArgumentType),
		const AttributeManager::Initializer& meta
	);

	// FieldInfo Getter, FieldInfo Setter
	template<typename ClassType, typename FieldType>
	void AddField(
		const std::string& name,
		typename FieldGetter<ClassType, FieldType, false>::Signature fieldGetter,
		typename FieldSetter<ClassType, FieldType, false>::Signature fieldSetter,
		const AttributeManager::Initializer& meta
	);

	const FieldInfo& GetField(const std::string& name) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Static Field
	///////////////////////////////////////////////////////////////////
	// Method Getter, Method Setter
	template<typename ClassType, typename FieldType, typename GetterType, typename SetterType>
	void AddStaticField(
		const std::string& name,
		GetterType getter,
		SetterType setter,
		const AttributeManager::Initializer& meta
	);

	// Method Getter, FieldInfo Setter
	template<typename ClassType, typename FieldType, typename GetterType>
	void AddStaticField(
		const std::string& name,
		GetterType getter,
		FieldType* fieldSetter,
		const AttributeManager::Initializer& meta
	);

	// FieldInfo Getter, Method Setter
	template<typename ClassType, typename FieldType, typename SetterType>
	void AddStaticField(
		const std::string& name,
		FieldType* fieldGetter,
		SetterType setter,
		const AttributeManager::Initializer& meta
	);

	// FieldInfo Getter, FieldInfo Setter
	template<typename ClassType, typename FieldType>
	void AddStaticField(
		const std::string& name,
		FieldType* fieldGetter,
		FieldType* fieldSetter,
		const AttributeManager::Initializer& meta
	);

	const FieldInfo& GetStaticField(const std::string& name) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Method
	///////////////////////////////////////////////////////////////////
	template<typename MethodType>
	void AddMethod(
		const std::string& name,
		MethodType method,
		const Invokable::ParamNameList& paramNames,
		const AttributeManager::Initializer& meta
	);

	const MethodInfo& GetMethod(const std::string& name);
	const MethodInfo& GetMethod(const std::string& name, const InvokableSignature& signature);

	///////////////////////////////////////////////////////////////////
	/////////////////////	Static Method
	///////////////////////////////////////////////////////////////////
	template<typename ClassType, typename FunctionType>
	void AddStaticMethod(
		const std::string& name,
		FunctionType function,
		const AttributeManager::Initializer& meta
	);

	const MethodInfo& GetStaticMethod(const std::string& name);
	const MethodInfo& GetStaticMethod(const std::string& name, const InvokableSignature& signature);

	///////////////////////////////////////////////////////////////////
	/////////////////////	Enum
	///////////////////////////////////////////////////////////////////
	template<typename EnumType>
	void SetEnum(const std::string& name, const typename EnumContainer<EnumType>::Initializer& initalizer);
};
END_META_NAMESPACE

#include "../../Impl/TypeData.hpp"