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

#pragma once

#include "GLM/fwd.hpp"
#include "GLM/ext/vector_float2.hpp" // glm::vec3
#include "GLM/ext/vector_float3.hpp" // glm::vec3
#include "GLM/ext/vector_float4.hpp" // glm::vec3
#include "GLM/ext/matrix_float4x4.hpp" // glm::vec3
#include "GLM/detail/type_quat.hpp"

namespace Horizon
{
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Mat4 = glm::mat4;

	using float2 = glm::vec2;
	using float3 = glm::vec3;
	using float4 = glm::vec4;

	using uint2 = glm::u32vec2;
	using uint3 = glm::u32vec3;
	using uint4 = glm::u32vec4;

	using int2 = glm::i32vec2;
	using int3 = glm::i32vec3;
	using int4 = glm::i32vec4;

	using matrix2x2 = glm::mat2;
	using matrix3x3 = glm::mat3;
	using matrix4x4 = glm::mat4;

	using matrix = matrix4x4;
	using quaternion = glm::quat;
}

