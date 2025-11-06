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

	void Texture2D::GenMipmap() const
	{
		if (m_Desc.Width <= 0 || m_Desc.Height <= 0)
			return;

		GL_THROW_INFO(glGenerateMipmap(GetTexType()));
	}

VISIONGAL_OPENGL_NAMESPACE_END

