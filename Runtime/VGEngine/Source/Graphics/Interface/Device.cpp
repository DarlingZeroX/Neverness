#include "Graphics/Interface/Device.h"
#include "Graphics/OpenGL/Texture2D.h"

namespace VisionGal::VGFX
{
	Ref<ITexture> CreateTextureFromMemory(const TextureDesc& desc)
	{
		return OpenGL::Texture2D::CreateFromMemory(desc);;
	}
} 