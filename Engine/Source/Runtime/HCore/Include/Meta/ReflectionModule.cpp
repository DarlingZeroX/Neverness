/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ReflectionModule.cpp
** --------------------------------------------------------------------------*/

#include "pch.h"

#include "ReflectionModule.h"

BEGIN_META_NAMESPACE
ReflectionModule::ReflectionModule(ReflectionDatabase& db)
	: db(db) { }
END_META_NAMESPACE