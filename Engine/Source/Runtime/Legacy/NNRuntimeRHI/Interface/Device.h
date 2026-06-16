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
#include "../Include/VGRHIConfig.h"
#include <NNCore/Interface/HCore.h>
#include "Texture.h"

namespace NN::Runtime::VGFX
{
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API NN::Ref<ITexture> CreateTextureFromMemory(const TextureDesc& desc);
} 