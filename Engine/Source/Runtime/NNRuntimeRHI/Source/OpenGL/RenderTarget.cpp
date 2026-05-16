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

#include "OpenGL/RenderTarget.h"
#include "OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN
	Ref<RenderTarget2D> RenderTarget2D::Create(unsigned int width, unsigned int height)
	{
		auto rt = MakeRef<RenderTarget2D>();
		rt->CreateImp(width, height);

		return rt;
	}

	void RenderTarget2D::CopyToTexture(VGFX::ITexture* texture)
	{
		m_FrameBuffer->Bind();

		Texture2D* glTexture = dynamic_cast<Texture2D*>(texture);
		glTexture->Bind();

		GL_THROW_INFO(glCopyTexSubImage2D(GL_TEXTURE_2D,  // 目标纹理类型
			0,              // mipmap 级别
			0, 0,           // 目标纹理偏移
			0, 0,           // 源帧缓冲区域左下角
			m_ColorFBT->Width(), m_ColorFBT->Height())); // 复制区域尺寸

		m_FrameBuffer->Unbind();
	} 

	void RenderTarget2D::BindColorAttachments()
	{
		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
		GL_THROW_INFO(glDrawBuffers(1, attachments));
	}

	bool RenderTarget2D::CreateImp(unsigned int width, unsigned int height)
	{
		m_FrameBuffer = MakeRef<FrameBuffer>();
		m_FrameBuffer->Bind();

		m_ColorFBT = FrameBufferTexture::Create(width, height, FrameBufferTexture::Type::COLOR);
		m_ColorFBT->AttachToFrameBuffer();

		BindColorAttachments();

		m_DepthFBT = FrameBufferDepth::Create(width, height);
		m_DepthFBT->AttachToFrameBuffer();

		if (m_FrameBuffer->CheckStatus() == false)
		{
			m_FrameBuffer->Unbind();
			return false;
		}

		m_FrameBuffer->Unbind();
		return true;
	}



VISIONGAL_OPENGL_NAMESPACE_END