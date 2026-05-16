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

#include "Render/Texture2D.h"
#include <NNKernel/Interface/HVector.h>

namespace VisionGal
{
	Texture2D::Texture2D()
		:m_Texture(nullptr)
	{
		
	}

	Texture2D::Texture2D(Ref<VGFX::ITexture> texture)
		:m_Texture(texture)
	{
		SetTexture(texture);
	}

	int Texture2D::Width()
	{
		if (const auto* desc = GetTextureDesc())
			return desc->Width;
		return 0;
	}

	int Texture2D::Height()
	{
		if (const auto* desc = GetTextureDesc())
			return desc->Height;
		return 0;
	}

	float2 Texture2D::Size()
	{
		return float2(Width(), Height());
	}

	const VGFX::TextureDesc* Texture2D::GetTextureDesc()
	{
		if (m_Texture)
			return &m_Texture->GetDesc();

		return nullptr;
	}

	Ref<VGFX::ITexture>& Texture2D::GetTexture()
	{
		return m_Texture;
	}

	void Texture2D::SetTexture(const Ref<VGFX::ITexture>& texture)
	{
		m_Texture = texture;
	}
}