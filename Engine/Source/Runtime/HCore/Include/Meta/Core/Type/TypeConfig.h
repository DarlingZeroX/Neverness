/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeConfig.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "TypeID.h"
#include "../MetaTraits.h"

#include <type_traits>

// Gets the type ID of a given expression
//#define TypeIDOf(expr)                                            \
//    META_NAMESPACE::TypeIDs<                                        \
//        META_NAMESPACE::CleanedType<                                \
//            typename META_NAMESPACE::traits::RemoveArray<expr>::type \
//        >                                                         \
//    >::ID                                                         \

#define TypeIDOf(expr)                                                  \
    META_NAMESPACE::TypeIDQuery::GetTypeID(                              \
       &typeid(  META_NAMESPACE::CleanedType<                            \
            typename META_NAMESPACE::traits::RemoveArray<expr>::type    \
        > )                                                             \
    )                                                                   \

#define TypeOf(expr)  META_NAMESPACE::Type::Get<expr>()

// Converts the resulting type of the given expression to a meta::Type instance
#define decltypeof(expr) TypeOf( decltype( expr ) )

BEGIN_META_NAMESPACE

template<typename T>
using CleanedType =
typename std::remove_cv<
	typename std::remove_reference< T >::type
>::type;

END_META_NAMESPACE
