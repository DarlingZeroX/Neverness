#pragma once
#include "../../Type.h"
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE

class HMETA_API ParameterInfo
{
public:
	struct ConstructArgs
	{
		Type paramType;
		std::string_view name;
		unsigned position;
	};
public:
	//template<class ParameterType>
	//ParameterInfo(const std::string& name);

	ParameterInfo(Type paramType, const std::string& name);

	ParameterInfo(ConstructArgs args);
public:
	Type GetParameterType() const;

	const std::string& GetName() const;

	unsigned GetPosition() const;

	//bool HasDefaultValue() const;

	//Variant GetDefaultValue() const;

private:
	Type m_type;

	std::string m_name;

	unsigned m_position;
};

END_META_NAMESPACE

#include "../../Impl/ParameterInfo.hpp"
