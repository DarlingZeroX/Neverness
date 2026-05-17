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
#include "../EngineConfig.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include <NNRuntimeRHI/Interface/Texture.h>

namespace NN::Runtime {

	class VG_ENGINE_API TextureConverter {
	public:
		// 现有方法...
		static VGFX::TexturePixels ToRGB(const VGFX::TexturePixels& texture);
		static VGFX::TexturePixels ToRGBWithAlphaBlending(const VGFX::TexturePixels& rgbaTexture,
			unsigned char backgroundR = 255, unsigned char backgroundG = 255, unsigned char backgroundB = 255);

		// 新增：垂直翻转纹理
		static VGFX::TexturePixels FlipY(const VGFX::TexturePixels& texture);

		// 修改保存方法，添加flipY参数
		static bool SaveAsJPG(const VGFX::TexturePixels& texture, const char* filename, int quality = 90,
			bool alphaBlend = false, bool flipY = true,  // 默认开启Y轴翻转
			unsigned char bgR = 255, unsigned char bgG = 255, unsigned char bgB = 255);

		// 新增：单独保存方法，可以控制更多参数
		static bool SaveAsJPGWithOptions(const VGFX::TexturePixels& texture, const char* filename,
			int quality = 90, bool alphaBlend = false, bool flipY = true,
			unsigned char bgR = 255, unsigned char bgG = 255, unsigned char bgB = 255);
	};
}