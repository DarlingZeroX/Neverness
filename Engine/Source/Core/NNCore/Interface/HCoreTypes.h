// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
//#include <cstdint>
#include <filesystem>
#include "../Include/Math/GLM/fwd.hpp"
 
#undef max
#undef min

#ifndef NULL
#define NULL    (0)
#endif

namespace NN::Core
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

    typedef std::uint64_t        HResult;

    using HPath = std::filesystem::path;

#ifdef HALF_FLOAT_ENABLED
	typedef half_float::half     float16;
#endif
}

// see example below
#define BITFLAG_ENUM_CLASS_HELPER( ENUMCLASSTYPE )                                                                                                              \
inline ENUMCLASSTYPE operator | ( ENUMCLASSTYPE lhs, const ENUMCLASSTYPE & rhs )                                                                                \
{                                                                                                                                                               \
    return (ENUMCLASSTYPE)( static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs ) | static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( rhs ) );            \
}                                                                                                                                                               \
                                                                                                                                                                \
inline ENUMCLASSTYPE & operator |= ( ENUMCLASSTYPE & lhs, const ENUMCLASSTYPE & rhs )                                                                           \
{                                                                                                                                                               \
    return lhs = (ENUMCLASSTYPE)( static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs ) | static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( rhs ) );      \
}                                                                                                                                                               \
                                                                                                                                                                \
inline ENUMCLASSTYPE operator & ( ENUMCLASSTYPE lhs, const ENUMCLASSTYPE & rhs )                                                                                \
{                                                                                                                                                               \
    return (ENUMCLASSTYPE)( static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs )& static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( rhs ) );             \
}                                                                                                                                                               \
                                                                                                                                                                \
inline ENUMCLASSTYPE & operator &= ( ENUMCLASSTYPE & lhs, const ENUMCLASSTYPE & rhs )                                                                           \
{                                                                                                                                                               \
    return lhs = (ENUMCLASSTYPE)( static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs )& static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( rhs ) );       \
}                                                                                                                                                               \
                                                                                                                                                                \
inline ENUMCLASSTYPE operator ~ ( ENUMCLASSTYPE lhs )                                                                                                           \
{                                                                                                                                                               \
    return (ENUMCLASSTYPE)( ~static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs ) );                                                                       \
}                                                                                                                                                               \
                                                                                                                                                                \
inline bool operator == ( ENUMCLASSTYPE lhs, const std::underlying_type_t<ENUMCLASSTYPE> & rhs )                                                                \
{                                                                                                                                                               \
    return static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs ) == rhs;                                                                                    \
}                                                                                                                                                               \
                                                                                                                                                                \
inline bool operator != ( ENUMCLASSTYPE lhs, const std::underlying_type_t<ENUMCLASSTYPE> & rhs )                                                                \
{                                                                                                                                                               \
    return static_cast<std::underlying_type_t<ENUMCLASSTYPE>>( lhs ) != rhs;                                                                                    \
}
// usage example:
// enum class vaResourceBindSupportFlags : uint32
// {
//     None                        = 0,
//     VertexBuffer                = (1 << 0),
//     IndexBuffer                 = (1 << 1),
//     ConstantBuffer              = (1 << 2),
//     ShaderResource              = (1 << 3),
//     RenderTarget                = (1 << 4),
//     DepthStencil                = (1 << 5),
//     UnorderedAccess             = (1 << 6),
// };
//
// BITFLAG_ENUM_CLASS_HELPER( vaResourceBindSupportFlags ); // <- and now you can just do "vaResourceBindSupportFlags::VertexBuffer | vaResourceBindSupportFlags::UnorderedAccess