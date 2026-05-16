// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "HCoreTypes.h"

namespace Horizon::HGI
{

	/// Texture formats

	/// This enumeration describes available texture formats and generally mirrors DXGI_FORMAT enumeration.
	/// The table below provides detailed information on each format. Most of the formats are widely supported
	/// by all modern APIs (DX10+, OpenGL3.3+ and OpenGLES3.0+). Specific requirements are additionally indicated.
	/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format">DXGI_FORMAT enumeration on MSDN</a>,
	///     <a href = "https://www.opengl.org/wiki/Image_Format">OpenGL Texture Formats</a>
	enum class HTextureFormat : uint32_t
	{
		/// Unknown format
		UNKNOWN = 0,

		/// Four-component 128-bit typeless format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32A32_TYPELESS. OpenGL does not have direct counterpart, GL_RGBA32F is used.
		RGBA32_TYPELESS,

		/// Four-component 128-bit floating-point format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32A32_FLOAT. OpenGL counterpart: GL_RGBA32F.
		RGBA32_FLOAT,

		/// Four-component 128-bit unsigned-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32A32_UINT. OpenGL counterpart: GL_RGBA32UI.
		RGBA32_UINT,

		/// Four-component 128-bit signed-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32A32_SINT. OpenGL counterpart: GL_RGBA32I.
		RGBA32_SINT,

		/// Three-component 96-bit typeless format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32_TYPELESS. OpenGL does not have direct counterpart, GL_RGB32F is used.
		/// \warning This format has weak hardware support and is not recommended
		RGB32_TYPELESS,

		/// Three-component 96-bit floating-point format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32_FLOAT. OpenGL counterpart: GL_RGB32F.
		/// \warning This format has weak hardware support and is not recommended
		RGB32_FLOAT,

		/// Three-component 96-bit unsigned-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32_UINT. OpenGL counterpart: GL_RGB32UI.
		/// \warning This format has weak hardware support and is not recommended
		RGB32_UINT,

		/// Three-component 96-bit signed-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32B32_SINT. OpenGL counterpart: GL_RGB32I.
		/// \warning This format has weak hardware support and is not recommended
		RGB32_SINT,

		/// Four-component 64-bit typeless format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_TYPELESS. OpenGL does not have direct counterpart, GL_RGBA16F is used.
		RGBA16_TYPELESS,

		/// Four-component 64-bit half-precision floating-point format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_FLOAT. OpenGL counterpart: GL_RGBA16F.
		RGBA16_FLOAT,

		/// Four-component 64-bit unsigned-normalized-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_UNORM. OpenGL counterpart: GL_RGBA16. \n
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		RGBA16_UNORM,

		/// Four-component 64-bit unsigned-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_UINT. OpenGL counterpart: GL_RGBA16UI.
		RGBA16_UINT,

		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// Four-component 64-bit signed-normalized-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_SNORM. OpenGL counterpart: GL_RGBA16_SNORM. \n
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		RGBA16_SNORM,

		/// Four-component 64-bit signed-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16B16A16_SINT. OpenGL counterpart: GL_RGBA16I.
		RGBA16_SINT,

		/// Two-component 64-bit typeless format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32_TYPELESS. OpenGL does not have direct counterpart, GL_RG32F is used.
		RG32_TYPELESS,

		/// Two-component 64-bit floating-point format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32_FLOAT. OpenGL counterpart: GL_RG32F.
		RG32_FLOAT,

		/// Two-component 64-bit unsigned-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32_UINT. OpenGL counterpart: GL_RG32UI.
		RG32_UINT,

		/// Two-component 64-bit signed-integer format with 32-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R32G32_SINT. OpenGL counterpart: GL_RG32I.
		RG32_SINT,

		/// Two-component 64-bit typeless format with 32-bits for R channel and 8 bits for G channel. \n
		/// D3D counterpart: DXGI_FORMAT_R32G8X24_TYPELESS. OpenGL does not have direct counterpart, GL_DEPTH32F_STENCIL8 is used.
		R32G8X24_TYPELESS,

		/// Two-component 64-bit format with 32-bit floating-point depth channel and 8-bit stencil channel. \n
		/// D3D counterpart: DXGI_FORMAT_D32_FLOAT_S8X24_UINT. OpenGL counterpart: GL_DEPTH32F_STENCIL8.
		D32_FLOAT_S8X24_UINT,

		/// Two-component 64-bit format with 32-bit floating-point R channel and 8+24-bits of typeless data. \n
		/// D3D counterpart: DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS. OpenGL does not have direct counterpart, GL_DEPTH32F_STENCIL8 is used.
		R32_FLOAT_X8X24_TYPELESS,

		/// Two-component 64-bit format with 32-bit typeless data and 8-bit G channel. \n
		/// D3D counterpart: DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
		/// \warning This format is currently not implemented in OpenGL version
		X32_TYPELESS_G8X24_UINT,

		/// Four-component 32-bit typeless format with 10 bits for RGB and 2 bits for alpha channel. \n
		/// D3D counterpart: DXGI_FORMAT_R10G10B10A2_TYPELESS. OpenGL does not have direct counterpart, GL_RGB10_A2 is used.
		RGB10A2_TYPELESS,

		/// Four-component 32-bit unsigned-normalized-integer format with 10 bits for each color and 2 bits for alpha channel. \n
		/// D3D counterpart: DXGI_FORMAT_R10G10B10A2_UNORM. OpenGL counterpart: GL_RGB10_A2.
		RGB10A2_UNORM,

		/// Four-component 32-bit unsigned-integer format with 10 bits for each color and 2 bits for alpha channel. \n
		/// D3D counterpart: DXGI_FORMAT_R10G10B10A2_UINT. OpenGL counterpart: GL_RGB10_A2UI.
		RGB10A2_UINT,

		/// Three-component 32-bit format encoding three partial precision channels using 11 bits for red and green and 10 bits for blue channel. \n
		/// D3D counterpart: DXGI_FORMAT_R11G11B10_FLOAT. OpenGL counterpart: GL_R11F_G11F_B10F.
		R11G11B10_FLOAT,

		/// Four-component 32-bit typeless format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_TYPELESS. OpenGL does not have direct counterpart, GL_RGBA8 is used.
		RGBA8_TYPELESS,

		/// Four-component 32-bit unsigned-normalized-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_UNORM. OpenGL counterpart: GL_RGBA8.
		RGBA8_UNORM,

		/// Four-component 32-bit unsigned-normalized-integer sRGB format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB. OpenGL counterpart: GL_SRGB8_ALPHA8.
		RGBA8_UNORM_SRGB,

		/// Four-component 32-bit unsigned-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_UINT. OpenGL counterpart: GL_RGBA8UI.
		RGBA8_UINT,

		/// Four-component 32-bit signed-normalized-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_SNORM. OpenGL counterpart: GL_RGBA8_SNORM.
		RGBA8_SNORM,

		/// Four-component 32-bit signed-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8B8A8_SINT. OpenGL counterpart: GL_RGBA8I.
		RGBA8_SINT,

		/// Two-component 32-bit typeless format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_TYPELESS. OpenGL does not have direct counterpart, GL_RG16F is used.
		RG16_TYPELESS,

		/// Two-component 32-bit half-precision floating-point format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_FLOAT. OpenGL counterpart: GL_RG16F.
		RG16_FLOAT,

		/// Two-component 32-bit unsigned-normalized-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_UNORM. OpenGL counterpart: GL_RG16. \n
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		RG16_UNORM,

		/// Two-component 32-bit unsigned-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_UINT. OpenGL counterpart: GL_RG16UI.
		RG16_UINT,

		/// Two-component 32-bit signed-normalized-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_SNORM. OpenGL counterpart: GL_RG16_SNORM. \n
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		RG16_SNORM,

		/// Two-component 32-bit signed-integer format with 16-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R16G16_SINT. OpenGL counterpart: GL_RG16I.
		RG16_SINT,

		/// Single-component 32-bit typeless format. \n
		/// D3D counterpart: DXGI_FORMAT_R32_TYPELESS. OpenGL does not have direct counterpart, GL_R32F is used.
		R32_TYPELESS,

		/// Single-component 32-bit floating-point depth format. \n
		/// D3D counterpart: DXGI_FORMAT_D32_FLOAT. OpenGL counterpart: GL_DEPTH_COMPONENT32F.
		D32_FLOAT,

		/// Single-component 32-bit floating-point format. \n
		/// D3D counterpart: DXGI_FORMAT_R32_FLOAT. OpenGL counterpart: GL_R32F.
		R32_FLOAT,

		/// Single-component 32-bit unsigned-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R32_UINT. OpenGL counterpart: GL_R32UI.
		R32_UINT,

		/// Single-component 32-bit signed-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R32_SINT. OpenGL counterpart: GL_R32I.
		R32_SINT,

		/// Two-component 32-bit typeless format with 24 bits for R and 8 bits for G channel. \n
		/// D3D counterpart: DXGI_FORMAT_R24G8_TYPELESS. OpenGL does not have direct counterpart, GL_DEPTH24_STENCIL8 is used.
		R24G8_TYPELESS,

		/// Two-component 32-bit format with 24 bits for unsigned-normalized-integer depth and 8 bits for stencil. \n
		/// D3D counterpart: DXGI_FORMAT_D24_UNORM_S8_UINT. OpenGL counterpart: GL_DEPTH24_STENCIL8.
		D24_UNORM_S8_UINT,

		/// Two-component 32-bit format with 24 bits for unsigned-normalized-integer data and 8 bits of unreferenced data. \n
		/// D3D counterpart: DXGI_FORMAT_R24_UNORM_X8_TYPELESS. OpenGL does not have direct counterpart, GL_DEPTH24_STENCIL8 is used.
		R24_UNORM_X8_TYPELESS,

		/// Two-component 32-bit format with 24 bits of unreferenced data and 8 bits of unsigned-integer data. \n
		/// D3D counterpart: DXGI_FORMAT_X24_TYPELESS_G8_UINT
		/// \warning This format is currently not implemented in OpenGL version
		X24_TYPELESS_G8_UINT,

		/// Two-component 16-bit typeless format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_TYPELESS. OpenGL does not have direct counterpart, GL_RG8 is used.
		RG8_TYPELESS,

		/// Two-component 16-bit unsigned-normalized-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_UNORM. OpenGL counterpart: GL_RG8.
		RG8_UNORM,

		/// Two-component 16-bit unsigned-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_UINT. OpenGL counterpart: GL_RG8UI.
		RG8_UINT,

		/// Two-component 16-bit signed-normalized-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_SNORM. OpenGL counterpart: GL_RG8_SNORM.
		RG8_SNORM,

		/// Two-component 16-bit signed-integer format with 8-bit channels. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_SINT. OpenGL counterpart: GL_RG8I.
		RG8_SINT,

		/// Single-component 16-bit typeless format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_TYPELESS. OpenGL does not have direct counterpart, GL_R16F is used.
		R16_TYPELESS,

		/// Single-component 16-bit half-precision floating-point format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_FLOAT. OpenGL counterpart: GL_R16F.
		R16_FLOAT,

		/// Single-component 16-bit unsigned-normalized-integer depth format. \n
		/// D3D counterpart: DXGI_FORMAT_D16_UNORM. OpenGL counterpart: GL_DEPTH_COMPONENT16.
		D16_UNORM,

		/// Single-component 16-bit unsigned-normalized-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_UNORM. OpenGL counterpart: GL_R16.
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		R16_UNORM,

		/// Single-component 16-bit unsigned-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_UINT. OpenGL counterpart: GL_R16UI.
		R16_UINT,

		/// Single-component 16-bit signed-normalized-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_SNORM. OpenGL counterpart: GL_R16_SNORM. \n
		/// [GL_EXT_texture_norm16]: https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_norm16.txt
		/// OpenGLES: [GL_EXT_texture_norm16][] extension is required
		R16_SNORM,

		/// Single-component 16-bit signed-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R16_SINT. OpenGL counterpart: GL_R16I.
		R16_SINT,

		/// Single-component 8-bit typeless format. \n
		/// D3D counterpart: DXGI_FORMAT_R8_TYPELESS. OpenGL does not have direct counterpart, GL_R8 is used.
		R8_TYPELESS,

		/// Single-component 8-bit unsigned-normalized-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R8_UNORM. OpenGL counterpart: GL_R8.
		R8_UNORM,

		/// Single-component 8-bit unsigned-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R8_UINT. OpenGL counterpart: GL_R8UI.
		R8_UINT,

		/// Single-component 8-bit signed-normalized-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R8_SNORM. OpenGL counterpart: GL_R8_SNORM.
		R8_SNORM,

		/// Single-component 8-bit signed-integer format. \n
		/// D3D counterpart: DXGI_FORMAT_R8_SINT. OpenGL counterpart: GL_R8I.
		R8_SINT,

		/// Single-component 8-bit unsigned-normalized-integer format for alpha only. \n
		/// D3D counterpart: DXGI_FORMAT_A8_UNORM
		/// \warning This format is not available in OpenGL
		A8_UNORM,

		/// Single-component 1-bit format. \n
		/// D3D counterpart: DXGI_FORMAT_R1_UNORM
		/// \warning This format is not available in OpenGL
		R1_UNORM,

		/// Three partial-precision floating pointer numbers sharing single exponent encoded into a 32-bit value. \n
		/// D3D counterpart: DXGI_FORMAT_R9G9B9E5_SHAREDEXP. OpenGL counterpart: GL_RGB9_E5.
		RGB9E5_SHAREDEXP,

		/// Four-component unsigned-normalized integer format analogous to UYVY encoding. \n
		/// D3D counterpart: DXGI_FORMAT_R8G8_B8G8_UNORM
		/// \warning This format is not available in OpenGL
		RG8_B8G8_UNORM,

		/// Four-component unsigned-normalized integer format analogous to YUY2 encoding. \n
		/// D3D counterpart: DXGI_FORMAT_G8R8_G8B8_UNORM
		/// \warning This format is not available in OpenGL
		G8R8_G8B8_UNORM,

		/// Four-component typeless block-compression format with 1:8 compression ratio.\n
		/// D3D counterpart: DXGI_FORMAT_BC1_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGB_S3TC_DXT1_EXT is used. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc1">BC1 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT1_Format">DXT1 on OpenGL.org </a>
		BC1_TYPELESS,

		/// Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel.
		/// The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC1_UNORM. OpenGL counterpart: GL_COMPRESSED_RGB_S3TC_DXT1_EXT.\n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc1">BC1 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT1_Format">DXT1 on OpenGL.org </a>
		BC1_UNORM,

		/// Four-component unsigned-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel. \n
		/// The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC1_UNORM_SRGB. OpenGL counterpart: GL_COMPRESSED_SRGB_S3TC_DXT1_EXT.\n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc1">BC1 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT1_Format">DXT1 on OpenGL.org </a>
		BC1_UNORM_SRGB,

		/// Four component typeless block-compression format with 1:4 compression ratio.\n
		/// D3D counterpart: DXGI_FORMAT_BC2_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT is used. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc2">BC2 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT3_Format">DXT3 on OpenGL.org </a>
		BC2_TYPELESS,

		/// Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 4 bits for low-coherent separate A channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC2_UNORM. OpenGL counterpart: GL_COMPRESSED_RGBA_S3TC_DXT3_EXT. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc2">BC2 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT3_Format">DXT3 on OpenGL.org </a>
		BC2_UNORM,

		/// Four-component signed-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 4 bits for low-coherent separate A channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC2_UNORM_SRGB. OpenGL counterpart: GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc2">BC2 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT3_Format">DXT3 on OpenGL.org </a>
		BC2_UNORM_SRGB,

		/// Four-component typeless block-compression format with 1:4 compression ratio.\n
		/// D3D counterpart: DXGI_FORMAT_BC3_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT is used. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc3">BC3 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT5_Format">DXT5 on OpenGL.org </a>
		BC3_TYPELESS,

		/// Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 8 bits for highly-coherent A channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC3_UNORM. OpenGL counterpart: GL_COMPRESSED_RGBA_S3TC_DXT5_EXT. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc3">BC3 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT5_Format">DXT5 on OpenGL.org </a>
		BC3_UNORM,

		/// Four-component unsigned-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 8 bits for highly-coherent A channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC3_UNORM_SRGB. OpenGL counterpart: GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT. \n
		/// [GL_EXT_texture_compression_s3tc]: https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
		/// OpenGL & OpenGLES: [GL_EXT_texture_compression_s3tc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc3">BC3 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/S3_Texture_Compression#DXT5_Format">DXT5 on OpenGL.org </a>
		BC3_UNORM_SRGB,

		/// One-component typeless block-compression format with 1:2 compression ratio. \n
		/// D3D counterpart: DXGI_FORMAT_BC4_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RED_RGTC1 is used. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc4">BC4 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC4_TYPELESS,

		/// One-component unsigned-normalized-integer block-compression format with 8 bits for R channel.
		/// The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:2 compression ratio against R8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC4_UNORM. OpenGL counterpart: GL_COMPRESSED_RED_RGTC1. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc4">BC4 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC4_UNORM,

		/// One-component signed-normalized-integer block-compression format with 8 bits for R channel.
		/// The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:2 compression ratio against R8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC4_SNORM. OpenGL counterpart: GL_COMPRESSED_SIGNED_RED_RGTC1. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc4">BC4 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC4_SNORM,

		/// Two-component typeless block-compression format with 1:2 compression ratio. \n
		/// D3D counterpart: DXGI_FORMAT_BC5_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RG_RGTC2 is used. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc5">BC5 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC5_TYPELESS,

		/// Two-component unsigned-normalized-integer block-compression format with 8 bits for R and 8 bits for G channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:2 compression ratio against RG8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC5_UNORM. OpenGL counterpart: GL_COMPRESSED_RG_RGTC2. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc5">BC5 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC5_UNORM,

		/// Two-component signed-normalized-integer block-compression format with 8 bits for R and 8 bits for G channel.
		/// The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:2 compression ratio against RG8 format. \n
		/// D3D counterpart: DXGI_FORMAT_BC5_SNORM. OpenGL counterpart: GL_COMPRESSED_SIGNED_RG_RGTC2. \n
		/// [GL_ARB_texture_compression_rgtc]: https://www.opengl.org/registry/specs/ARB/texture_compression_rgtc.txt
		/// OpenGL & OpenGLES: [GL_ARB_texture_compression_rgtc][] extension is required
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc5">BC5 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/Image_Format#Compressed_formats">Compressed formats on OpenGL.org </a>
		BC5_SNORM,

		/// Three-component 16-bit unsigned-normalized-integer format with 5 bits for blue, 6 bits for green, and 5 bits for red channel. \n
		/// D3D counterpart: DXGI_FORMAT_B5G6R5_UNORM
		/// \warning This format is not available until D3D11.1 and Windows 8. It is also not available in OpenGL
		B5G6R5_UNORM,

		/// Four-component 16-bit unsigned-normalized-integer format with 5 bits for each color channel and 1-bit alpha. \n
		/// D3D counterpart: DXGI_FORMAT_B5G5R5A1_UNORM
		/// \warning This format is not available until D3D11.1 and Windows 8. It is also not available in OpenGL
		B5G5R5A1_UNORM,

		/// Four-component 32-bit unsigned-normalized-integer format with 8 bits for each channel. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8A8_UNORM.
		/// \warning This format is not available in OpenGL
		BGRA8_UNORM,

		/// Four-component 32-bit unsigned-normalized-integer format with 8 bits for each color channel and 8 bits unused. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8X8_UNORM.
		/// \warning This format is not available in OpenGL
		BGRX8_UNORM,

		/// Four-component 32-bit 2.8-biased fixed-point format with 10 bits for each color channel and 2-bit alpha. \n
		/// D3D counterpart: DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM.
		/// \warning This format is not available in OpenGL
		R10G10B10_XR_BIAS_A2_UNORM,

		/// Four-component 32-bit typeless format with 8 bits for each channel. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8A8_TYPELESS.
		/// \warning This format is not available in OpenGL
		BGRA8_TYPELESS,

		/// Four-component 32-bit unsigned-normalized sRGB format with 8 bits for each channel. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8A8_UNORM_SRGB.
		/// \warning This format is not available in OpenGL.
		BGRA8_UNORM_SRGB,

		/// Four-component 32-bit typeless format that with 8 bits for each color channel, and 8 bits are unused. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8X8_TYPELESS.
		/// \warning This format is not available in OpenGL.
		BGRX8_TYPELESS,

		/// Four-component 32-bit unsigned-normalized sRGB format with 8 bits for each color channel, and 8 bits are unused. \n
		/// D3D counterpart: DXGI_FORMAT_B8G8R8X8_UNORM_SRGB.
		/// \warning This format is not available in OpenGL.
		BGRX8_UNORM_SRGB,

		/// Three-component typeless block-compression format. \n
		/// D3D counterpart: DXGI_FORMAT_BC6H_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT is used. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc6h-format">BC6H on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC6H_TYPELESS,

		/// Three-component unsigned half-precision floating-point format with 16 bits for each channel. \n
		/// D3D counterpart: DXGI_FORMAT_BC6H_UF16. OpenGL counterpart: GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc6h-format">BC6H on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC6H_UF16,

		/// Three-channel signed half-precision floating-point format with 16 bits per each channel. \n
		/// D3D counterpart: DXGI_FORMAT_BC6H_SF16. OpenGL counterpart: GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc6h-format">BC6H on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC6H_SF16,

		/// Three-component typeless block-compression format. \n
		/// D3D counterpart: DXGI_FORMAT_BC7_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGBA_BPTC_UNORM is used. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format">BC7 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC7_TYPELESS,

		/// Three-component block-compression unsigned-normalized-integer format with 4 to 7 bits per color channel and 0 to 8 bits of alpha. \n
		/// D3D counterpart: DXGI_FORMAT_BC7_UNORM. OpenGL counterpart: GL_COMPRESSED_RGBA_BPTC_UNORM. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format">BC7 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC7_UNORM,

		/// Three-component block-compression unsigned-normalized-integer sRGB format with 4 to 7 bits per color channel and 0 to 8 bits of alpha. \n
		/// D3D counterpart: DXGI_FORMAT_BC7_UNORM_SRGB. OpenGL counterpart: GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM. \n
		/// [GL_ARB_texture_compression_bptc]: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
		/// OpenGL: [GL_ARB_texture_compression_bptc][] extension is required. Not supported in at least OpenGLES3.1
		/// \sa <a href = "https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format">BC7 on MSDN </a>,
		///     <a href = "https://www.opengl.org/wiki/BPTC_Texture_Compression">BPTC Texture Compression on OpenGL.org </a>
		BC7_UNORM_SRGB,

		/// Helper member containing the total number of texture formats in the enumeration
		NUM_FORMATS
	};

}