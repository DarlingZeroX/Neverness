#include "pch.h"
#include "TypeID.h"
#include "../../ReflectionDatabase.h"

BEGIN_META_NAMESPACE
	TypeID TypeIDQuery::GetTypeID(const type_info* info)
	{
		return ReflectionDatabase::Instance().GetTypeID(info);
	}

END_META_NAMESPACE
