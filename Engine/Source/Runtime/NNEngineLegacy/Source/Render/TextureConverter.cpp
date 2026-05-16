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

#include "Render/TextureConverter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <NNKernel/Include/Stb/stb_image_write.h>

namespace VisionGal {
	VGFX::TexturePixels TextureConverter::ToRGB(const VGFX::TexturePixels& texture)
	{
		VGFX::TexturePixels result;
		result.Width = texture.Width;
		result.Height = texture.Height;
		result.NumComponents = 3;

		if (texture.NumComponents == 4) {
			int pixelCount = texture.Width * texture.Height;
			result.Data.resize(pixelCount * 3);

			for (int i = 0; i < pixelCount; ++i) {
				result.Data[i * 3] = texture.Data[i * 4];     // R
				result.Data[i * 3 + 1] = texture.Data[i * 4 + 1]; // G
				result.Data[i * 3 + 2] = texture.Data[i * 4 + 2]; // B
			}
		}
		else if (texture.NumComponents == 3) {
			// 已经是RGB，直接复制
			result.Data = texture.Data;
		}
		else {
			SDL_Log("Cannot convert %d-channel texture to RGB", texture.NumComponents);
		}

		return result;
	}

	VGFX::TexturePixels TextureConverter::ToRGBWithAlphaBlending(const VGFX::TexturePixels& rgbaTexture,
		unsigned char backgroundR, unsigned char backgroundG, unsigned char backgroundB)
	{
		VGFX::TexturePixels result;
		result.Width = rgbaTexture.Width;
		result.Height = rgbaTexture.Height;
		result.NumComponents = 3;

		if (rgbaTexture.NumComponents != 4) {
			SDL_Log("Texture is not RGBA format");
			return result;
		}

		int pixelCount = rgbaTexture.Width * rgbaTexture.Height;
		result.Data.resize(pixelCount * 3);

		for (int i = 0; i < pixelCount; ++i) {
			unsigned char r = rgbaTexture.Data[i * 4];
			unsigned char g = rgbaTexture.Data[i * 4 + 1];
			unsigned char b = rgbaTexture.Data[i * 4 + 2];
			unsigned char a = rgbaTexture.Data[i * 4 + 3];

			float alpha = a / 255.0f;

			// Alpha混合：前景色 * alpha + 背景色 * (1 - alpha)
			result.Data[i * 3] = static_cast<unsigned char>(r * alpha + backgroundR * (1 - alpha));
			result.Data[i * 3 + 1] = static_cast<unsigned char>(g * alpha + backgroundG * (1 - alpha));
			result.Data[i * 3 + 2] = static_cast<unsigned char>(b * alpha + backgroundB * (1 - alpha));
		}

		return result;
	}

	// 新增：垂直翻转纹理
	VGFX::TexturePixels TextureConverter::FlipY(const VGFX::TexturePixels& texture)
	{
		VGFX::TexturePixels result;
		result.Width = texture.Width;
		result.Height = texture.Height;
		result.NumComponents = texture.NumComponents;
		result.Data.resize(texture.Data.size());

		if (texture.Data.empty()) {
			return result;
		}

		int bytesPerPixel = texture.NumComponents;
		int rowSize = texture.Width * bytesPerPixel;

		for (int y = 0; y < texture.Height; ++y) {
			int srcY = texture.Height - 1 - y;  // 从底部开始读取
			int dstY = y;                      // 从顶部开始写入

			const unsigned char* srcRow = texture.Data.data() + srcY * rowSize;
			unsigned char* dstRow = result.Data.data() + dstY * rowSize;

			// 复制整行数据
			memcpy(dstRow, srcRow, rowSize);
		}

		return result;
	}

	// 修改后的保存方法（保持向后兼容）
	bool TextureConverter::SaveAsJPG(const VGFX::TexturePixels& texture, const char* filename, int quality,
		bool alphaBlend, bool flipY, unsigned char bgR, unsigned char bgG, unsigned char bgB)
	{
		return SaveAsJPGWithOptions(texture, filename, quality, alphaBlend, flipY, bgR, bgG, bgB);
	}

	// 新增：带完整选项的保存方法
	bool TextureConverter::SaveAsJPGWithOptions(const VGFX::TexturePixels& texture, const char* filename,
		int quality, bool alphaBlend, bool flipY,
		unsigned char bgR, unsigned char bgG, unsigned char bgB)
	{
		if (texture.Data.empty()) {
			SDL_Log("Texture data is empty");
			return false;
		}

		if (texture.Width <= 0 || texture.Height <= 0) {
			SDL_Log("Invalid texture dimensions: %dx%d", texture.Width, texture.Height);
			return false;
		}

		const VGFX::TexturePixels* processingTexture = &texture;
		VGFX::TexturePixels tempTexture1, tempTexture2;

		// 第一步：通道转换（如果需要）
		if (texture.NumComponents == 4) {
			if (alphaBlend) {
				tempTexture1 = ToRGBWithAlphaBlending(texture, bgR, bgG, bgB);
			}
			else {
				tempTexture1 = ToRGB(texture);
			}
			processingTexture = &tempTexture1;
		}
		else if (texture.NumComponents != 3) {
			SDL_Log("Unsupported texture format: %d channels", texture.NumComponents);
			return false;
		}

		// 第二步：Y轴翻转（如果需要）
		const VGFX::TexturePixels* finalTexture = processingTexture;
		if (flipY) {
			tempTexture2 = FlipY(*processingTexture);
			finalTexture = &tempTexture2;
		}

		// 第三步：保存为JPG
		bool success = stbi_write_jpg(filename, finalTexture->Width, finalTexture->Height, 3,
			finalTexture->Data.data(), quality) != 0;

		if (!success) {
			SDL_Log("Failed to save JPG: %s", filename);
		}

		return success;
	}
}