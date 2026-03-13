/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Invokable.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "../Type.h"
#include "TypeUnpacker.hpp"

namespace std
{
	template<>
	struct hash<META_NAMESPACE::InvokableSignature>
	{
		size_t operator()(
			const META_NAMESPACE::InvokableSignature& signature
			) const
		{
			hash<META_NAMESPACE::TypeID> hasher;

			size_t seed = 0;

			// combine the hash of all type IDs in the signature
			for (auto& type : signature)
				seed ^= hasher(type.GetID()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};
}

BEGIN_META_NAMESPACE
template<typename ...Types>
InvokableSignature Invokable::CreateSignature(void)
{
	static InvokableSignature signature;

	static auto initial = true;

	if (initial)
	{
		TypeUnpacker<Types...>::Apply(signature);

		initial = false;
	}

	return signature;
}
END_META_NAMESPACE