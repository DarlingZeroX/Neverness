/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ReflectionDatabase.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "ReflectionDatabase.h"

#include "Type.h"
//auto id = AllocateType( #type );    
#define REGISTER_NATIVE_TYPE(type)                    \
    {                                                 \
        auto id = RegisterType( &typeid(type),#type ); \
        auto &handle = types[ id ];                   \
                                                      \
        TypeInfo::Register<type>( id, handle, true ); \
    }                                                 \

#define REGISTER_NATIVE_TYPE_VARIANTS(type) \
    REGISTER_NATIVE_TYPE( type )            \
    REGISTER_NATIVE_TYPE( type* )           \
    REGISTER_NATIVE_TYPE( const type* )     \

#define REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(type)         \
    REGISTER_NATIVE_TYPE_VARIANTS( type )                   \
    types[ TypeIDOf( type ) ].SetArrayConstructor<type>( ); \

BEGIN_META_NAMESPACE
ReflectionDatabase::ReflectionDatabase(void)
	: types(1)
	, m_nextID(1)
{

}

void ReflectionDatabase::Initialize()
{
	types[InvalidTypeID].name = "UNKNOWN";

	// register all of the native type variants explicity, before
	// anything else
	REGISTER_NATIVE_TYPE_VARIANTS(void);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(int);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(unsigned int);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(bool);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(float);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(double);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(std::string);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(float2);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(float3);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(float4);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(matrix2x2);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(matrix3x3);
	REGISTER_NATIVE_TYPE_VARIANTS_W_ARRAY(matrix4x4);

	auto& stringType = types[TypeIDOf(std::string)];

	// explicitly add default constructors for string

	stringType.AddConstructor<std::string, false, false>({ });
	stringType.AddConstructor<std::string, false, true>({ });
}

///////////////////////////////////////////////////////////////////////

ReflectionDatabase::~ReflectionDatabase(void) { }

///////////////////////////////////////////////////////////////////////

TypeID ReflectionDatabase::GetTypeID(const type_info* info)
{
	auto result = typeInfos.find(info);
	if(result == typeInfos.end())
		return InvalidTypeID;

	return result->second.ID;
}

bool ReflectionDatabase::TypeDefined(const type_info* info)
{
	auto result = typeInfos.find(info);
	if (result == typeInfos.end())
		return false;

	return result->second.Defined;
}

void ReflectionDatabase::TypeSetDefined(const type_info* info)
{
	auto result = typeInfos.find(info);
	if (result == typeInfos.end())
		return;

	result->second.Defined = true;
}

ReflectionDatabase& ReflectionDatabase::Instance(void)
{
	static bool initialized = false;
	static ReflectionDatabase instance;

	if(!initialized)
	{
		initialized = true;
		instance.Initialize();
	}

	return instance;
}

TypeID ReflectionDatabase::RegisterType(const type_info* typeInfo, const std::string& fullName)
{
	TypeID id = AllocateType(fullName);

	typeInfos[typeInfo].ID = id;

	return id;
}

///////////////////////////////////////////////////////////////////////

TypeID ReflectionDatabase::AllocateType(const std::string& fullName)
{
	auto search = ids.find(fullName);

	// already defined
	if (search != ids.end())
		return InvalidTypeID;

	types.emplace_back(fullName);

	auto id = m_nextID++;

	ids[fullName] = id;

	return id;
}



///////////////////////////////////////////////////////////////////////

const MethodInfo& ReflectionDatabase::GetGlobalFunction(
	const std::string& name
)
{
	auto& base = globalFunctions[name];

	if (!base.size())
		return MethodInfo::Invalid();

	return base.begin()->second;
}

///////////////////////////////////////////////////////////////////////

const MethodInfo& ReflectionDatabase::GetGlobalFunction(
	const std::string& name,
	const InvokableSignature& signature
)
{
	auto& base = globalFunctions[name];

	auto search = base.find(signature);

	if (search == base.end())
		return MethodInfo::Invalid();

	return search->second;
}
END_META_NAMESPACE