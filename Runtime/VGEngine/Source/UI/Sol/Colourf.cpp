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

#include "Colourf.h"
#include <RmlUi/Core/ElementDocument.h>

namespace RmlSol {

	void RmlColourf::RegisterType(sol::state* lua)
	{
		// Rml::Colourf
		lua->new_usertype<Rml::Colourf>("RmlColourf",
			sol::constructors<Rml::Colourf()>(),
			"r", &Rml::Colourf::red,
			"g", &Rml::Colourf::green,
			"b", &Rml::Colourf::blue,
			"a", &Rml::Colourf::alpha,

			sol::meta_function::multiplication, [](const Rml::Colourf& lhs, float rhs) {
				return lhs * rhs;
			},
			sol::meta_function::addition, [](const Rml::Colourf& lhs, const Rml::Colourf& rhs) {
				return lhs + rhs;
			},
			sol::meta_function::subtraction, [](const Rml::Colourf& lhs, const Rml::Colourf& rhs) {
				return lhs - rhs;
			},
			sol::meta_function::equal_to, [](const Rml::Colourf& lhs, const Rml::Colourf& rhs) {
				return lhs == rhs;
			}
		);
	}
}
