// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <cstdint>
#include "../Math/HVector.h"
#include <filesystem>

#undef max
#undef min

#ifndef NULL
#define NULL    (0)
#endif

namespace Horizon
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