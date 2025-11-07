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
#include "TextureBase.h"
#include "../Interface/Texture.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class Texture2D :public TextureBase, public VGFX::ITexture
	{
	public:
		Texture2D() = default;
		Texture2D(Texture2D&) = delete;
		~Texture2D() override = default;

		GLenum GetTexType() const noexcept override;

		int Width() const noexcept { return m_Desc.Width; }
		int Height() const noexcept { return m_Desc.Height; }

		static Ref<Texture2D> CreateFromMemory(const VGFX::TextureDesc& desc);

		const VGFX::TextureDesc& GetDesc() override;
		void* GetShaderResourceView() override;
		bool ReadPixels(VGFX::TexturePixels& outPixels) override;
	protected:
		virtual void GenMipmap() const;

		bool CreateFromMemoryImp(const VGFX::TextureDesc& desc);
	protected:
		//unsigned int m_RendererID = 0;
		//unsigned int  m_Slot = 0;

		VGFX::TextureDesc m_Desc;
	};

VISIONGAL_OPENGL_NAMESPACE_END