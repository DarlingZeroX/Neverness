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

#include "OpenGL/Texture2D.h"
#include "OpenGL/ThrowMarco.h"

namespace NN::Runtime::OpenGL {

	GLenum Texture2D::GetTexType() const noexcept
	{
		return GL_TEXTURE_2D;
	}

	//bool Texture2D::CreateFromMemoryImp(const VGFX::TextureDesc& desc, int RowPitch, int BytesPerPixel)
	//{
	//	m_Desc = desc;
	//
	//	// 检查参数
	//	if (!desc.Data || desc.Width <= 0 || desc.Height <= 0 || RowPitch <= 0)
	//		return false;
	//
	//	GenTex();
	//	BindTex();
	//
	//	TexWrapping(GL_CLAMP_TO_EDGE);
	//	TexFlitering(GL_LINEAR);
	//
	//	/*
	//	 *pitch（surface->pitch）可能 != width * channels。直接把 surface->pixels 传给 glTexImage2D 会产生错位/条纹/倾斜。
	//	 即使 GL_UNPACK_ALIGNMENT = 1，如果 pitch 有额外字节，OpenGL 会按 row length 读取像素 —— 需要设置 GL_UNPACK_ROW_LENGTH 或者把像素拆行拷贝成紧凑缓冲区。
	//	 通道顺序错误会导致色彩错位（变成灰或色偏），比如 SDL 上的像素可能是 BGR，或平台字节序影响 packed formats。
	//	 某些 packed 32-bit 格式（ABGR/ARGB）上传时需要正确的 Type（大多数情况 GL_UNSIGNED_BYTE 就可以配合正确 Format 使用，但有些特例需要 GL_UNSIGNED_INT_8_8_8_8_REV）
	//	 */
	//	// ----------------------------------------------------------------------
	//	// ⭐ 方案 A：行拷贝到紧凑连续 buffer			把 surface 的每行拷贝到一个 tightly-packed 的临时缓冲区，然后上传。对所有格式安全，最少出错。
	//	// ----------------------------------------------------------------------
	//	const int rows = desc.Height;
	//	const int colsBytes = desc.Width * BytesPerPixel;
	//
	//	// 分配紧密 buffer
	//	std::vector<uint8_t> tightBuffer(rows * colsBytes);
	//
	//	const uint8_t* src = static_cast<const uint8_t*>(desc.Data);
	//	uint8_t* dst = tightBuffer.data();
	//
	//	for (int y = 0; y < rows; y++)
	//	{
	//		// 每一行：从 pitch 长度的行里取出实际有效数据
	//		memcpy(dst + y * colsBytes,
	//			src + y * RowPitch,
	//			colsBytes);
	//	}
	//
	//	// ----------------------------------------------------------------------
	//	// 上传纹理
	//	// ----------------------------------------------------------------------
	//	GL_THROW_INFO(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	//
	//	GL_THROW_INFO(glTexImage2D(
	//		GL_TEXTURE_2D,
	//		0,
	//		desc.InternalFormat,
	//		desc.Width,
	//		desc.Height,
	//		0,
	//		desc.Format,
	//		desc.Type,
	//		tightBuffer.data()      // <-- 现在是紧密数据
	//	));
	//
	//	GenMipmap();
	//	UnBindTex();
	//	return true;
	//}


	bool Texture2D::CreateFromMemoryImp(const VGFX::TextureDesc& desc)
	{
		m_Desc = desc;
	
		GenTex();
		BindTex();
	
		// 设置纹理参数
	
		TexWrapping(GL_CLAMP_TO_EDGE);
		TexFlitering(GL_LINEAR);
	
		// 上传纹理数据
		GL_THROW_INFO(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		GL_THROW_INFO(glTexImage2D(GL_TEXTURE_2D, 0, desc.InternalFormat, desc.Width, desc.Height, 0, desc.Format, desc.Type, desc.Data));
	
		GenMipmap();
	
		UnBindTex();
	
		return false;
	}

	NN::Ref<Texture2D> Texture2D::CreateFromMemory(const VGFX::TextureDesc& desc)
	{
		auto Tex = std::make_shared<Texture2D>();

		Tex->CreateFromMemoryImp(desc);

		return Tex;
	}

	//Ref<Texture2D> Texture2D::CreateFromMemory(const VGFX::TextureDesc& desc, int RowPitch, int BytesPerPixel)
	//{
	//	auto Tex = std::make_shared<Texture2D>();
	//
	//	Tex->CreateFromMemoryImp(desc, RowPitch, BytesPerPixel);
	//
	//	return Tex;
	//}

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

}

