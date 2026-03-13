#include "pch.h"
#include "TypeInfo.h"
#include "../../ReflectionDatabase.h"

BEGIN_META_NAMESPACE
	bool TypeInfo::TypeDefined(const type_info& info)
	{
		return ReflectionDatabase::Instance().TypeDefined(&info);
	}

	void TypeInfo::TypeSetDefined(const type_info& info)
	{
		return ReflectionDatabase::Instance().TypeSetDefined(&info);
	}

END_META_NAMESPACE
