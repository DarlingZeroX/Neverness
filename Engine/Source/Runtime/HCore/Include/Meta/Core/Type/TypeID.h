/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeData.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE
typedef unsigned TypeID;

const TypeID InvalidTypeID = 0;

//template<typename T>
//struct TypeIDs
//{
//	static TypeID ID;
//};

struct HMETA_API TypeIDQuery
{
	static TypeID GetTypeID(const type_info* info);
};

//template<typename T>
//TypeID TypeIDs<T>::ID = InvalidTypeID;
END_META_NAMESPACE
