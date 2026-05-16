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

#include "OpenGL/Buffer.h"
#include "OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	BufferGL::BufferGL(BufferGL&& buf) noexcept

	{
		m_RendererID = buf.m_RendererID;
		buf.m_RendererID = 0;
	}

	BufferGL& BufferGL::operator=(BufferGL&& buf) noexcept
	{
		Delete();
		m_RendererID = buf.m_RendererID;
		buf.m_RendererID = 0;
		return *this;
	}

	BufferGL::~BufferGL()
	{
		Delete();
	}
	 
	void BufferGL::Gen(int size)
	{
		H_ASSERT_NULL(m_RendererID);
		m_Size = size;
		GL_THROW_INFO(glGenBuffers(size, &m_RendererID));
	}

	void BufferGL::Bind(GLenum eTarget) const
	{
		H_ASSERT_NOT_NULL(m_RendererID);
		GL_THROW_INFO(glBindBuffer(eTarget, m_RendererID));
	}

	void BufferGL::Unbind(GLenum eTarget) const
	{
		GL_THROW_INFO(glBindBuffer(eTarget, 0));
	}

	void BufferGL::Delete()
	{
		if (m_RendererID != NULL)
		{
			GL_THROW_INFO(glDeleteBuffers(m_Size, &m_RendererID));
			m_RendererID = NULL;
		}
	}

	void BufferGL::SetData(GLenum eTarget, GLsizeiptr size, const void* data, GLenum usage) const
	{
		GL_THROW_INFO(glBufferData(eTarget, size, data, usage));
	}

	void BufferGL::SetStaticData(GLenum eTarget, GLsizeiptr size, const void* data) const
	{
		SetData(eTarget, size, data, GL_STATIC_DRAW);
	}

	void BufferGL::SetDynamicData(GLenum eTarget, GLsizeiptr size, const void* data) const
	{
		SetData(eTarget, size, data, GL_DYNAMIC_DRAW);
	}

	void BufferGL::SetStreamData(GLenum eTarget, GLsizeiptr size, const void* data) const
	{
		SetData(eTarget, size, data, GL_STREAM_DRAW);
	}

	GLuint BufferGL::GetRendererID() const noexcept
	{
		H_ASSERT_NOT_NULL(m_RendererID);
		return m_RendererID;
	}

VISIONGAL_OPENGL_NAMESPACE_END

