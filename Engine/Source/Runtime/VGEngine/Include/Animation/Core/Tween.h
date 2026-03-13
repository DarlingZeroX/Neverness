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
/**
	Tween below.
	Partly based on RmlUI http://github.com/mikke89/RmlUi
 */

#pragma once
#include "../../Interface/GameInterface.h"

namespace VisionGal {

	class Tween {
	public:
		enum Type { None, Back, Bounce, Circular, Cubic, Elastic, Exponential, Linear, Quadratic, Quartic, Quintic, Sine, Callback, Count };
		enum Direction { In = 1, Out = 2, InOut = 3 };
		using CallbackFnc = float (*)(float);

		Tween(Type type = Linear, Direction direction = Out);
		Tween(Type type_in, Type type_out);
		Tween(CallbackFnc callback, Direction direction = In);

		// Evaluate the Tweening function at point t in [0, 1].
		float operator()(float t) const;

		// Reverse direction of the tweening function.
		void reverse();

		bool operator==(const Tween& other) const;
		bool operator!=(const Tween& other) const;

		String to_string() const;

		static bool ParseTweenByString(const std::string& tween, Tween& out);
	private:
		float tween(Type type, float t) const;
		float in(float t) const;
		float out(float t) const;
		float in_out(float t) const;

		Type type_in = None;
		Type type_out = None;
		CallbackFnc callback = nullptr;
	};

} // namespace Rml
