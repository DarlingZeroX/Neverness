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
#include <vector>
#include <NNCore/Interface/HCoreTypes.h>

namespace NN::Runtime::VGFX
{
	struct TextureDesc
	{
		int Width;
		int Height;
		int InternalFormat;
		NN::Core::uint Format;
		NN::Core::uint Type;
		void* Data;
		unsigned int DataSize;
	};

	struct TexturePixels
	{
		int Width = 0;
		int Height = 0;
		int NumComponents = 0;
		std::vector<unsigned char> Data;
	};

	struct ITexture
	{
	public:
		virtual ~ITexture() = default;
 
		virtual const TextureDesc& GetDesc() = 0;
		virtual void* GetShaderResourceView() = 0;
		virtual bool ReadPixels(TexturePixels& outPixels) = 0;
	};
}