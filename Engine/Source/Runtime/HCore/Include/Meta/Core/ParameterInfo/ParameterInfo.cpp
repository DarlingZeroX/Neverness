#include "pch.h"
#include "ParameterInfo.h"

BEGIN_META_NAMESPACE
ParameterInfo::ParameterInfo(Type paramType, const std::string& name)
	:m_name(name)
{
}

ParameterInfo::ParameterInfo(ConstructArgs args)
	:
	m_type(args.paramType),
	m_name(args.name),
	m_position(args.position)
{
}

Type ParameterInfo::GetParameterType() const
{
	return m_type;
}

const std::string& ParameterInfo::GetName() const
{
	return m_name;
}

unsigned ParameterInfo::GetPosition() const
{
	return m_position;
}

//bool ParameterInfo::HasDefaultValue() const
//{
//	throw "TODO";
//}

//Variant ParameterInfo::GetDefaultValue() const
//{
//	throw "TODO";
//	return Variant{};
//}

END_META_NAMESPACE