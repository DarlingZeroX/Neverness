#pragma once
#include "../../EngineConfig.h"
#include "../../Core/Core.h"

namespace VisionGal::VGFX
{
	struct TextureDesc
	{
		int Width;
		int Height;
		int InternalFormat;
		uint Format;
		uint Type;
		void* Data;
		unsigned int DataSize;
	};

	//struct TexturePixels
	//{
	//	int Width = 0;
	//	int Height = 0;
	//	int NumComponents = 0;
	//	std::vector<unsigned char> Data;
	//};

	struct ITexture
	{
	public:
		virtual ~ITexture() = default;
 
		virtual const TextureDesc& GetDesc() = 0;
		virtual void* GetShaderResourceView() = 0;
		//virtual bool ReadPixels(TexturePixels& outPixels) = 0;
	};
}