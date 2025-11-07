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

#include "Graphics/OpenGL/Texture2D.h"
#include "Graphics/OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	GLenum Texture2D::GetTexType() const noexcept
	{
		return GL_TEXTURE_2D;
	}

	bool Texture2D::CreateFromMemoryImp(const VGFX::TextureDesc& desc)
	{
		m_Desc = desc;

		GenTex();
		BindTex();

		// 设置纹理参数

		TexWrapping(GL_CLAMP_TO_EDGE);
		TexFlitering(GL_LINEAR);

		// 上传纹理数据
		GL_THROW_INFO(glTexImage2D(GL_TEXTURE_2D, 0, desc.InternalFormat, desc.Width, desc.Height, 0, desc.Format, desc.Type, desc.Data));

		GenMipmap();

		UnBindTex();

		return false;
	}

	Ref<Texture2D> Texture2D::CreateFromMemory(const VGFX::TextureDesc& desc)
	{
		auto Tex = std::make_shared<Texture2D>();

		Tex->CreateFromMemoryImp(desc);

		return Tex;
	}

	const VGFX::TextureDesc& Texture2D::GetDesc()
	{
		return m_Desc;
	}

	void* Texture2D::GetShaderResourceView()
	{
		return reinterpret_cast<void*>(GetRendererID());;
	}

	bool Texture2D::ReadPixels(VGFX::TexturePixels& outPixels)
	{
		if (m_Desc.Format != GL_RGBA)
		{
			throw "Only support reading RGBA format texture pixels.";
		}

		outPixels.NumComponents = 4;
		outPixels.Width = m_Desc.Width;
		outPixels.Height = m_Desc.Height;

		if (outPixels.Width < 1 || outPixels.Height < 1)
			return false;

		const int byteSize = outPixels.Width * outPixels.Height * outPixels.NumComponents;
		outPixels.Data.resize(byteSize);

		glBindTexture(GL_TEXTURE_2D, GetRendererID());
		glGetTexImage(GL_TEXTURE_2D, 0, m_Desc.Format, GL_UNSIGNED_BYTE, outPixels.Data.data());
		glBindTexture(GL_TEXTURE_2D, 0);

		bool result = true;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			result = false;
			H_LOG_ERROR("Could not capture screenshot, got GL error: 0x%x", err);
		}

		if (!result)
			return false;

		return true;
	}

	void Texture2D::GenMipmap() const
	{
		if (m_Desc.Width <= 0 || m_Desc.Height <= 0)
			return;

		GL_THROW_INFO(glGenerateMipmap(GetTexType()));
	}

VISIONGAL_OPENGL_NAMESPACE_END

