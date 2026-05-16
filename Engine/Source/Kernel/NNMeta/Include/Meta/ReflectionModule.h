/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ReflectionModule.h
** --------------------------------------------------------------------------*/

#pragma once
#include "MetaCommonDefinitions.h"
#include "ReflectionDatabase.h"

#define DECLARE_REFLECTION_MODULE(module)                            \
    namespace meta_generated                                         \
    {                                                                \
        class Module##module : public META_NAMESPACE::ReflectionModule \
        {                                                            \
        public:                                                      \
             Module##module(META_NAMESPACE::ReflectionDatabase &db);   \
            ~Module##module(void);                                   \
        };                                                           \
    }                                                                \

#define UsingModule(module) meta_generated::Module##module _##module( db );

BEGIN_META_NAMESPACE
class HMETA_API ReflectionModule
{
public:
	ReflectionModule(ReflectionDatabase& db);

protected:
	ReflectionDatabase& db;
};
END_META_NAMESPACE