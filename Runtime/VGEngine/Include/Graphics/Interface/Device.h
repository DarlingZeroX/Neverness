#pragma once
#include "../../EngineConfig.h"
#include "../../Core/Core.h"
#include "Texture.h"

namespace VisionGal::VGFX
{
	VG_ENGINE_API Ref<ITexture> CreateTextureFromMemory(const TextureDesc& desc);
} 