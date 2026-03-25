/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Sol/Utilities.h"

namespace RmlSol {

	sol::object PushVariant(sol::this_state& L, const Rml::Variant* var)
	{
        sol::state_view lua(L);

        if (!var) {
            return sol::make_object(lua, sol::nil);
        }

        switch (var->GetType())
        {
        case Rml::Variant::BOOL:
            return sol::make_object(lua, var->Get<bool>());
        case Rml::Variant::BYTE:
        case Rml::Variant::CHAR:
        case Rml::Variant::INT:
            return sol::make_object(lua, var->Get<int>());
        case Rml::Variant::INT64:
            return sol::make_object(lua, var->Get<int64_t>());
        case Rml::Variant::UINT:
            return sol::make_object(lua, var->Get<unsigned int>());
        case Rml::Variant::UINT64:
            return sol::make_object(lua, var->Get<uint64_t>());
        case Rml::Variant::FLOAT:
        case Rml::Variant::DOUBLE:
            return sol::make_object(lua, var->Get<double>());
        case Rml::Variant::STRING: {
            const auto& s = var->GetReference<Rml::String>();
            return sol::make_object(lua, s);
        }
        case Rml::Variant::COLOURB:
            return sol::make_object(lua, new Rml::Colourb(var->Get<Rml::Colourb>())); // 需绑定 usertype
        case Rml::Variant::COLOURF:
            return sol::make_object(lua, new Rml::Colourf(var->Get<Rml::Colourf>()));
        case Rml::Variant::VECTOR2:
            return sol::make_object(lua, new Rml::Vector2f(var->Get<Rml::Vector2f>()));
        case Rml::Variant::VOIDPTR:
            return sol::make_object(lua, var->Get<void*>());
        default:
            return sol::make_object(lua, sol::nil);
        }
	}

	sol::object PushIndex(sol::this_state& L, int index)
	{
		return sol::make_object(L, index + 1);
	}
}