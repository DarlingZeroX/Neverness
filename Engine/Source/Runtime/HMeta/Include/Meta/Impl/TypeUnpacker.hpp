/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** TypeUnpacker.hpp
** --------------------------------------------------------------------------*/

#pragma once

BEGIN_META_NAMESPACE
template<class... Types>
struct TypeUnpacker { };

template<>
struct TypeUnpacker<>
{
	static void Apply(Type::List& types) { }
};

template<class First, class... Types>
struct TypeUnpacker<First, Types...>
{
	static void Apply(Type::List& types)
	{
		types.emplace_back(TypeOf(First));

		TypeUnpacker<Types...>::Apply(types);
	}
};
END_META_NAMESPACE