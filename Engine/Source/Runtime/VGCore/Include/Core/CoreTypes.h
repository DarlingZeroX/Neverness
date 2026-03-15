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
#include <HCore/Interface/HCoreTypes.h>
#include <HCore/Include/Scene/HEntityInterface.h>
#include <string>
#include <SDL3/SDL.h>

namespace VisionGal
{
	typedef std::int8_t          sbyte;
	typedef std::uint8_t         byte;

	typedef std::int8_t          int8;
	typedef std::uint8_t         uint8;

	typedef std::int16_t         int16;
	typedef std::uint16_t        uint16;

	typedef std::int32_t         int32;
	typedef std::uint32_t        uint32;
	typedef std::uint32_t        uint;       // for compatibility with shaders

	typedef std::int64_t         int64;
	typedef std::uint64_t        uint64;

	typedef int VGResult;

	using float2 = Horizon::float2;
	using float3 = Horizon::float3;
	using float4 = Horizon::float4;

	using uint2 = Horizon::uint2;
	using uint3 = Horizon::uint3;
	using uint4 = Horizon::uint4;

	using int2 = Horizon::int2;
	using int3 = Horizon::int3;
	using int4 = Horizon::int4;

	using matrix2x2 = Horizon::matrix2x2;
	using matrix3x3 = Horizon::matrix3x3;
	using matrix4x4 = Horizon::matrix4x4;

	using matrix = matrix4x4;
	using quaternion = Horizon::quaternion;

	using WindowID = SDL_WindowID;

	using String = std::string;
	using VGPath = String;

	typedef Horizon::HEntityID VGActorID;
	using VGComponentID = Horizon::uint64;
	constexpr VGActorID ACTOR_ID_NULL = 0;
}
