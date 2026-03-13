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

#include "OpenGL/FrameBuffer.h"
#include "OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	FrameBuffer::FrameBuffer()
		:
		m_RendererID(0)
	{
		CreateImp();
	}

	FrameBuffer::~FrameBuffer()
	{
		Delete();
	}

	Ref<FrameBuffer> FrameBuffer::Create()
	{
		Ref<FrameBuffer> fb = CreateRef<FrameBuffer>();

		return fb;
	}

	void FrameBuffer::Gen()
	{
		GL_THROW_INFO(glGenFramebuffers(1, &m_RendererID));
	}

	void FrameBuffer::Bind() const
	{
		GL_THROW_INFO(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
	}

	void FrameBuffer::Unbind() const
	{
		GL_THROW_INFO(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void FrameBuffer::Delete()
	{
		GL_THROW_INFO(glDeleteFramebuffers(1, &m_RendererID));
	}

	bool FrameBuffer::CheckStatus()
	{
		return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void FrameBuffer::DrawBuffers(unsigned int size)
	{
		// - 告诉OpenGL我们将要使用(帧缓冲的)哪种颜色附件来进行渲染
		std::vector<GLuint> attachments;
		attachments.resize(size);

		for (unsigned int i = 0; i < size; ++i)
		{
			attachments[i] = GL_COLOR_ATTACHMENT0 + size;
		}

		GL_THROW_INFO(glDrawBuffers(size, attachments.data()));
	}

	void FrameBuffer::CreateImp()
	{
		Gen();
		Bind();
	} 

	void FrameBufferTexture::CreateFBTInline(unsigned int width, unsigned int height, Type type, unsigned int slot)
	{
		m_Desc.Width = width;
		m_Desc.Height = height;

		m_Type = type;
		SetSlot(slot);
		m_FrameBufferSlot = slot;

		GenTex();
		Bind();

		m_FrameBufferBindType = GL_COLOR_ATTACHMENT0 + m_FrameBufferSlot;

		//Generate buffer
		switch (type)
		{
		case Type::COLOR:
			GL_THROW_INFO(glTexImage2D(
				GL_TEXTURE_2D, 0,
				GL_RGBA,
				width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE,
				NULL));
			m_Desc.Format = GL_RGBA;
			m_Desc.InternalFormat = GL_RGBA;
			break;
		case Type::NORMAL:
			GL_THROW_INFO(glTexImage2D(
				GL_TEXTURE_2D, 0,
				GL_RGB16F,
				width, height, 0,
				GL_RGB, GL_UNSIGNED_BYTE,
				NULL));
			m_Desc.Format = GL_RGB;
			m_Desc.InternalFormat = GL_RGB16F;
			break;
		case Type::ALBEDO_SPEC:
			GL_THROW_INFO(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL));
			m_Desc.Format = GL_RGBA;
			m_Desc.InternalFormat = GL_RGBA;
			break;
		case Type::DEPTH24_STENCIL8:
			GL_THROW_INFO(glTexImage2D(
				GL_TEXTURE_2D, 0,
				GL_DEPTH24_STENCIL8,
				width, height, 0,
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
				NULL
			));
			m_FrameBufferBindType = GL_DEPTH_STENCIL_ATTACHMENT;
			m_Desc.InternalFormat = GL_DEPTH24_STENCIL8;
			m_Desc.Format = GL_DEPTH_STENCIL;
			break;
		case Type::DEPTH_COMPONENT:
			GL_THROW_INFO(glTexImage2D(
				GL_TEXTURE_2D, 0,
				GL_DEPTH_COMPONENT,
				width, height, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT,
				NULL
			));
			m_FrameBufferBindType = GL_DEPTH_ATTACHMENT;
			m_Desc.InternalFormat = GL_DEPTH_COMPONENT;
			m_Desc.Format = GL_DEPTH_COMPONENT;
			break;
		default:
			throw "Attachment type not exist";
			break;
		}

		//Set texture paramerter
		switch (type)
		{
		case Type::DEPTH24_STENCIL8:
		case Type::DEPTH_COMPONENT:
			//深度和模板缓冲必须邻近取值
			TexFlitering(GL_NEAREST);
			//用于深度缓冲超过纹理的默认深度
			TexWrapping(GL_CLAMP_TO_BORDER);
			break;
		default:
			TexFlitering(GL_LINEAR);
			break;
		}

		Unbind();
	}

	Ref<FrameBufferTexture> FrameBufferTexture::Create(unsigned int width, unsigned int height, Type type, unsigned int slot)
	{
		Ref<FrameBufferTexture> fbt = CreateRef<FrameBufferTexture>();
		fbt->CreateFBTInline(width, height, type, slot);

		return fbt;
	}

	void FrameBufferTexture::AttachToFrameBuffer()
	{
		/// <summary>
		/// glFramebufferTexture2D(target,attachment,textarget,texture,level)
		/// target：我们所创建的帧缓冲类型的目标（绘制、读取或两者都有）。
		/// attachment：我们所附加的附件的类型。现在我们附加的是一个颜色附件。需要注意，最后的那个0是暗示我们可以附加1个以上颜色的附件。我们会在后面的教程中谈到。
		///	textarget：你希望附加的纹理类型。
		///	texture：附加的实际纹理。
		///	level：Mipmap level。我们设置为0。
		/// </summary>

		GL_THROW_INFO(glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			m_FrameBufferBindType,
			GL_TEXTURE_2D,
			GetRendererID(),
			0
		));
	}

	int FrameBufferTexture::Width()
	{
		return GetDesc().Width;
	}

	int FrameBufferTexture::Height()
	{
		return GetDesc().Height;
	}


	/// <summary>
	/// RenderBuffer Object Attachment
	/// </summary>
	FrameBufferDepth::FrameBufferDepth()
	{
		m_RenderBufferID = 0;
		m_Type = Type::DEPTH24_STENCIL8;
		m_AttachType = 0;
		m_StorageType = 0;
	}

	FrameBufferDepth::~FrameBufferDepth()
	{
		Delete();
	}

	Ref<FrameBufferDepth> FrameBufferDepth::Create(unsigned int width, unsigned int height, Type type)
	{
		Ref<FrameBufferDepth> fbt = CreateRef<FrameBufferDepth>();
		fbt->CreateImp(width, height, type);

		return fbt;
	}

	bool FrameBufferDepth::CreateImp(
		unsigned int width,
		unsigned int height,
		Type type
	)
	{
		m_Type = type;
		Gen();
		Bind();

		switch (type)
		{
		case Type::DEPTH24_STENCIL8:
			m_AttachType = GL_DEPTH_STENCIL_ATTACHMENT;
			break;
		case Type::DEPTH_COMPONENT:
			m_AttachType = GL_DEPTH_ATTACHMENT;
			break;
		default:
			Unbind();
			return false;
		}
		m_StorageType = (GLenum)type;
		SetStorage(width, height);

		Unbind();

		return true;
	}

	void FrameBufferDepth::AttachToFrameBuffer() const
	{
		GL_THROW_INFO(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			m_AttachType,
			GL_RENDERBUFFER,
			m_RenderBufferID
		));
	}

	void FrameBufferDepth::Bind() const
	{
		GL_THROW_INFO(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID));
	}

	void FrameBufferDepth::Unbind() const
	{
		GL_THROW_INFO(glBindRenderbuffer(GL_RENDERBUFFER, 0));
	}

	void FrameBufferDepth::Delete()
	{
		if (m_RenderBufferID != 0)
		{
			GL_THROW_INFO(glDeleteRenderbuffers(1, &m_RenderBufferID));
			m_RenderBufferID = 0;
		}
	}

	void FrameBufferDepth::SetStorage(int width, int height)
	{
		GL_THROW_INFO(glRenderbufferStorage(GL_RENDERBUFFER, m_StorageType, width, height));
	}

	unsigned int FrameBufferDepth::GetRendererID() const
	{
		return m_RenderBufferID;
	}

	GLenum FrameBufferDepth::GetAttachType() const
	{
		return m_AttachType;
	}

	GLenum FrameBufferDepth::GetStorageType() const
	{
		return m_StorageType;
	}

	void FrameBufferDepth::Gen()
	{
		GL_THROW_INFO(glGenRenderbuffers(1, &m_RenderBufferID));
	}

VISIONGAL_OPENGL_NAMESPACE_END

