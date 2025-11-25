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

#include "Asset/TextureAsset.h"
#include <SDL3_image/SDL_image.h>
#include "Core/Core.h"
#include "Core/VFS.h"

namespace VisionGal
{
	ImageFormat SDLImgFormat2VGImgFormat(SDL_PixelFormat format)
	{
		switch (format)
		{
        case SDL_PIXELFORMAT_UNKNOWN: return ImageFormat::UNKNOWN;
        case SDL_PIXELFORMAT_INDEX1LSB: return ImageFormat::INDEX1LSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_4321, 0, 1, 0), */
        case SDL_PIXELFORMAT_INDEX1MSB: return ImageFormat::INDEX1MSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_1234, 0, 1, 0), */
        case SDL_PIXELFORMAT_INDEX2LSB: return ImageFormat::INDEX2LSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX2, SDL_BITMAPORDER_4321, 0, 2, 0), */
        case SDL_PIXELFORMAT_INDEX2MSB: return ImageFormat::INDEX2MSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX2, SDL_BITMAPORDER_1234, 0, 2, 0), */
        case SDL_PIXELFORMAT_INDEX4LSB: return ImageFormat::INDEX4LSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_4321, 0, 4, 0), */
        case SDL_PIXELFORMAT_INDEX4MSB: return ImageFormat::INDEX4MSB;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_1234, 0, 4, 0), */
        case SDL_PIXELFORMAT_INDEX8: return ImageFormat::INDEX8;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX8, 0, 0, 8, 1), */
        case SDL_PIXELFORMAT_RGB332: return ImageFormat::RGB332;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED8, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_332, 8, 1), */
        case SDL_PIXELFORMAT_XRGB4444: return ImageFormat::XRGB4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_4444, 12, 2), */
        case SDL_PIXELFORMAT_XBGR4444: return ImageFormat::XBGR4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_4444, 12, 2), */
        case SDL_PIXELFORMAT_XRGB1555: return ImageFormat::XRGB1555;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_1555, 15, 2), */
        case SDL_PIXELFORMAT_XBGR1555: return ImageFormat::XBGR1555;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_1555, 15, 2), */
        case SDL_PIXELFORMAT_ARGB4444: return ImageFormat::ARGB4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_4444, 16, 2), */
        case SDL_PIXELFORMAT_RGBA4444: return ImageFormat::RGBA4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_4444, 16, 2), */
        case SDL_PIXELFORMAT_ABGR4444: return ImageFormat::ABGR4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_4444, 16, 2), */
        case SDL_PIXELFORMAT_BGRA4444: return ImageFormat::BGRA4444;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_4444, 16, 2), */
        case SDL_PIXELFORMAT_ARGB1555: return ImageFormat::ARGB1555;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_1555, 16, 2), */
        case SDL_PIXELFORMAT_RGBA5551: return ImageFormat::RGBA5551;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_5551, 16, 2), */
        case SDL_PIXELFORMAT_ABGR1555: return ImageFormat::ABGR1555;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_1555, 16, 2), */
        case SDL_PIXELFORMAT_BGRA5551: return ImageFormat::BGRA5551;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_5551, 16, 2), */
        case SDL_PIXELFORMAT_RGB565: return ImageFormat::RGB565;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_565, 16, 2), */
        case SDL_PIXELFORMAT_BGR565: return ImageFormat::BGR565;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_565, 16, 2), */
        case SDL_PIXELFORMAT_RGB24: return ImageFormat::RGB24;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0, 24, 3), */
        case SDL_PIXELFORMAT_BGR24: return ImageFormat::BGR24;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0, 24, 3), */
        case SDL_PIXELFORMAT_XRGB8888: return ImageFormat::XRGB8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_8888, 24, 4), */
        case SDL_PIXELFORMAT_RGBX8888: return ImageFormat::RGBX8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX, SDL_PACKEDLAYOUT_8888, 24, 4), */
        case SDL_PIXELFORMAT_XBGR8888: return ImageFormat::XBGR8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_8888, 24, 4), */
        case SDL_PIXELFORMAT_BGRX8888: return ImageFormat::BGRX8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX, SDL_PACKEDLAYOUT_8888, 24, 4), */
        case SDL_PIXELFORMAT_ARGB8888: return ImageFormat::ARGB8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_8888, 32, 4), */
        case SDL_PIXELFORMAT_RGBA8888: return ImageFormat::RGBA8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_8888, 32, 4), */
        case SDL_PIXELFORMAT_ABGR8888: return ImageFormat::ABGR8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_8888, 32, 4), */
        case SDL_PIXELFORMAT_BGRA8888: return ImageFormat::BGRA8888;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_8888, 32, 4), */
        case SDL_PIXELFORMAT_XRGB2101010: return ImageFormat::XRGB2101010;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_2101010, 32, 4), */
        case SDL_PIXELFORMAT_XBGR2101010: return ImageFormat::XBGR2101010;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_2101010, 32, 4), */
        case SDL_PIXELFORMAT_ARGB2101010: return ImageFormat::ARGB2101010;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_2101010, 32, 4), */
        case SDL_PIXELFORMAT_ABGR2101010: return ImageFormat::ABGR2101010;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_2101010, 32, 4), */
        case SDL_PIXELFORMAT_RGB48: return ImageFormat::RGB48;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_RGB, 0, 48, 6), */
        case SDL_PIXELFORMAT_BGR48: return ImageFormat::BGR48;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_BGR, 0, 48, 6), */
        case SDL_PIXELFORMAT_RGBA64: return ImageFormat::RGBA64;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_RGBA, 0, 64, 8), */
        case SDL_PIXELFORMAT_ARGB64: return ImageFormat::ARGB64;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_ARGB, 0, 64, 8), */
        case SDL_PIXELFORMAT_BGRA64: return ImageFormat::BGRA64;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_BGRA, 0, 64, 8), */
        case SDL_PIXELFORMAT_ABGR64: return ImageFormat::ABGR64;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_ABGR, 0, 64, 8), */
        case SDL_PIXELFORMAT_RGB48_FLOAT: return ImageFormat::RGB48_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_RGB, 0, 48, 6), */
        case SDL_PIXELFORMAT_BGR48_FLOAT: return ImageFormat::BGR48_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_BGR, 0, 48, 6), */
        case SDL_PIXELFORMAT_RGBA64_FLOAT: return ImageFormat::RGBA64_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_RGBA, 0, 64, 8), */
        case SDL_PIXELFORMAT_ARGB64_FLOAT: return ImageFormat::ARGB64_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_ARGB, 0, 64, 8), */
        case SDL_PIXELFORMAT_BGRA64_FLOAT: return ImageFormat::BGRA64_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_BGRA, 0, 64, 8), */
        case SDL_PIXELFORMAT_ABGR64_FLOAT: return ImageFormat::ABGR64_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_ABGR, 0, 64, 8), */
        case SDL_PIXELFORMAT_RGB96_FLOAT: return ImageFormat::RGB96_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_RGB, 0, 96, 12), */
        case SDL_PIXELFORMAT_BGR96_FLOAT: return ImageFormat::BGR96_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_BGR, 0, 96, 12), */
        case SDL_PIXELFORMAT_RGBA128_FLOAT: return ImageFormat::RGBA128_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_RGBA, 0, 128, 16), */
        case SDL_PIXELFORMAT_ARGB128_FLOAT: return ImageFormat::ARGB128_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_ARGB, 0, 128, 16), */
        case SDL_PIXELFORMAT_BGRA128_FLOAT: return ImageFormat::BGRA128_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_BGRA, 0, 128, 16), */
        case SDL_PIXELFORMAT_ABGR128_FLOAT: return ImageFormat::ABGR128_FLOAT;
         /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_ABGR, 0, 128, 16), */

        case SDL_PIXELFORMAT_YV12: return ImageFormat::YV12;      /**< Planar mode: Y + V + U  (3 planes) */
         /* SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'), */
        case SDL_PIXELFORMAT_IYUV: return ImageFormat::IYUV;      /**< Planar mode: Y + U + V  (3 planes) */
         /* SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'), */
        case SDL_PIXELFORMAT_YUY2: return ImageFormat::YUY2;    /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
         /* SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'), */
        case SDL_PIXELFORMAT_UYVY: return ImageFormat::UYVY;      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
         /* SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'), */
        case SDL_PIXELFORMAT_YVYU: return ImageFormat::YVYU;     /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
         /* SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'), */
        case SDL_PIXELFORMAT_NV12: return ImageFormat::NV12;     /**< Planar mode: Y + U/V interleaved  (2 planes) */
         /* SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'), */
        case SDL_PIXELFORMAT_NV21: return ImageFormat::NV21;     /**< Planar mode: Y + V/U interleaved  (2 planes) */
         /* SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'), */
        case SDL_PIXELFORMAT_P010: return ImageFormat::P010;   /**< Planar mode: Y + U/V interleaved  (2 planes) */
         /* SDL_DEFINE_PIXELFOURCC('P', '0', '1', '0'), */
        case SDL_PIXELFORMAT_EXTERNAL_OES: return ImageFormat::EXTERNAL_OES;     /**< Android video texture format */
         /* SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ') */

        case SDL_PIXELFORMAT_MJPG: return ImageFormat::MJPG;     /**< Motion JPEG */

		default:
            return ImageFormat::UNKNOWN;
		}
	}


	bool TextureAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		SDL_Surface* surface = nullptr;

		VFS::SafeReadFileFromVFS(path, [&](const VFS::DataRef& data) {

			const size_t i_ext = path.rfind('.');
			String extension = (i_ext == String::npos ? String() : path.substr(i_ext + 1));

			auto CreateSurface = [&]() {
				return IMG_LoadTyped_IO(SDL_IOFromMem(data->data(), data->size()), 1, extension.c_str());
				};

			surface = CreateSurface();
			return 0;
			});

		if (!surface) {
			H_LOG_ERROR("The texture failed to load: %s", path.c_str());
			return false;
		}

		auto texAsset = CreateRef<TextureAsset>();

		texAsset->Width = surface->w;
		texAsset->Height = surface->h;
		texAsset->Data = surface->pixels;

		// SDLFormat -> VGImageFormat
		texAsset->Format = SDLImgFormat2VGImgFormat(surface->format);

		// 因为 SDL_Surface 的每行数据可能有 padding（pitch），以及 通道顺序（RGB vs BGR）可能不完全匹配
		// 所以需要把这些数据保留下来，以创建纹理时使用
		// ⭐ 最重要：把 pitch 和 SDL_Surface* 保存下来
		texAsset->RowPitch = surface->pitch;        // 逐行字节数（可能 != width * channels）
		texAsset->UserPtr = surface;               // 后续 GL 上传可以取 surface->format / pitch
		texAsset->BytesPerPixel = SDL_BYTESPERPIXEL(surface->format);

		// 释放回调
		texAsset->FreeCallback = [surface]() {
			SDL_DestroySurface(surface);
			};

		H_LOG_INFO(
			"Texture loaded: %s [W=%d H=%d Format=%d SDLFormat=%d Pitch=%d]",
			path.c_str(),
			surface->w,
			surface->h,
			texAsset->Format,
			surface->format,
			surface->pitch
		);

		asset = texAsset;
		return true;
	}

    TextureAsset::~TextureAsset()
    {
        if (FreeCallback != nullptr)
            FreeCallback();

        FreeCallback = nullptr;
    };
}
