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

#include "OpenGL/Mesh.h"
#include "OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	VertexBuffer::~VertexBuffer()
	{
		Delete();
	}

	void VertexBuffer::Bind() const
	{
		m_glBuffer.Bind(GL_ARRAY_BUFFER);
	}

	void VertexBuffer::Unbind() const
	{
		m_glBuffer.Unbind(GL_ARRAY_BUFFER);
	}

	void VertexBuffer::Delete()
	{
		m_glBuffer.Delete();
	}

	unsigned int VertexBuffer::GetRendererID() const noexcept
	{
		return m_glBuffer.GetRendererID();
	} 

	void VertexBuffer::UpdateAllData(void* data)
	{
		// 更新VBO中的顶点数据
		Bind();
		GL_THROW_INFO(glBufferSubData(GL_ARRAY_BUFFER, 0, m_Size, data));
	}

	void VertexBuffer::NativeCreate(const void* data, unsigned int size)
	{
		m_Size = size;

		m_glBuffer.Gen();
		m_glBuffer.Bind(GL_ARRAY_BUFFER);
		m_glBuffer.SetStaticData(GL_ARRAY_BUFFER, size, data);
	}


	IndexBuffer::~IndexBuffer()
	{
		Delete();
	}

	void IndexBuffer::Create(const void* indices, unsigned int stride, unsigned int count)
	{
		m_Count = count;

		m_glBuffer.Gen();
		m_glBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
		m_glBuffer.SetStaticData(GL_ELEMENT_ARRAY_BUFFER, m_Count * stride, indices);
	}

	void IndexBuffer::NativeCreate(const void* data, unsigned int size)
	{
		m_Count = size / sizeof(unsigned int);

		m_glBuffer.Gen();
		m_glBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
		m_glBuffer.SetStaticData(GL_ELEMENT_ARRAY_BUFFER, size, data);
	}

	void IndexBuffer::Bind() const
	{
		m_glBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
	}

	void IndexBuffer::Unbind() const
	{
		m_glBuffer.Unbind(GL_ELEMENT_ARRAY_BUFFER);
	}

	void IndexBuffer::Delete()
	{
		m_glBuffer.Delete();
	}

	unsigned int IndexBuffer::GetCount() noexcept
	{
		return m_Count;
	}

	unsigned int IndexBuffer::GetRendererID() const noexcept
	{
		return m_glBuffer.GetRendererID();
	}


	void StaticMesh::Render()
	{
		Draw();
	}

	void StaticMesh::Bind() const
	{
		m_VertexArray.Bind();
	}

	void StaticMesh::Draw()
	{
		Bind();
		GL_THROW_INFO(glDrawElements(GL_TRIANGLES, m_Indices->GetCount(), GL_UNSIGNED_INT, nullptr));
		UnBind();
	}

	void StaticMesh::UnBind() const
	{
		GL_THROW_INFO(glBindVertexArray(0));
	}

VISIONGAL_OPENGL_NAMESPACE_END


