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
#include "../../Interface/Mesh.h"
#include "OpenGL.h"
#include "Buffer.h"
#include "VertexArray.h"

namespace NN::Runtime::OpenGL {

	class VG_RHI_API VertexBuffer :public Bindable, public Renderable
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) noexcept = default;
		VertexBuffer& operator=(VertexBuffer&&) noexcept = default;
		~VertexBuffer() override;

		template<class V>
		VertexBuffer(std::vector<V>& vertices)
		{
			Create(vertices);
		}

		VertexBuffer(void* data, unsigned int size)
		{
			NativeCreate(data, size);
		}
	public:
		virtual void Bind() const override; 
		virtual void Unbind() const override;
		void Delete();
		virtual unsigned int GetRendererID() const noexcept override;

		void UpdateAllData(void* data);
	private:
		template<class V>
		void Create(std::vector<V>& vertices)
		{
			Create(vertices.data(), vertices.size());
		}

		template<class V>
		void Create(const V* vertices, unsigned int count)
		{
			m_Stride = sizeof(V);
			m_Size = count * m_Stride;

			NativeCreate(vertices, m_Size);
		}

		void NativeCreate(const void* data, unsigned int size);
	private:
		unsigned int m_Stride;
		unsigned int m_Size;

		BufferGL m_glBuffer;
	};

	class VG_RHI_API IndexBuffer :public Bindable, public Renderable
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&&) noexcept = default;
		IndexBuffer& operator=(IndexBuffer&&) noexcept = default;
		~IndexBuffer() override;

		template<class V>
		IndexBuffer(std::vector<V>& indices)
		{
			Create(indices);
		}

		IndexBuffer(const void* data, unsigned int size)
		{
			NativeCreate(data, size);
		}
	public:
		template<class V>
		void Create(const V* indices, unsigned int count)
		{
			unsigned int stride = sizeof(V);

			Create(indices, stride, count);
		}

		void Create(const void* indices, unsigned int stride, unsigned int count);

		template<class V>
		void Create(std::vector<V>& indices)
		{
			Create(indices.data(), indices.size());
		}

		void NativeCreate(const void* data, unsigned int size);
	public:
		virtual void Bind() const override;
		virtual void Unbind() const override;
		void Delete();
		unsigned int GetCount() noexcept;
		virtual unsigned int GetRendererID() const noexcept override;
	protected:
		unsigned int m_Count;
		BufferGL m_glBuffer;
	};

	class VG_RHI_API StaticMesh: public VGFX::IStaticMesh
	{
	public:
		//StaticMesh() = default;
		StaticMesh(StaticMesh& mesh) = delete;
		StaticMesh& operator=(StaticMesh& mesh) = delete;
		~StaticMesh() override = default;

		template<class VERTEX, class INDEX, class ELEMENT>
		StaticMesh(
			std::vector<VERTEX>& vertices,
			std::vector<INDEX>& indices,
			std::vector<ELEMENT*> elements
		)
		{
			m_VertexArray.Create();
			m_Vertices = std::make_unique<VertexBuffer>(vertices);
			m_Indices = std::make_unique<IndexBuffer>(indices);
			m_VertexArray.PushElement(elements);
			m_VertexArray.Unbind();

			//Initialize(
			//	vertices.data(),
			//	vertices.size() * sizeof(VERTEX),
			//	indices.data(),
			//	indices.size() * sizeof(INDEX),
			//	elements
			//);
		}

		StaticMesh(void* vertexData, unsigned int vertexSize, void* indexData, unsigned int indexSize, std::vector<VGFX::VertexElement::IElement*> elements)
		{
			Initialize(vertexData, vertexSize, indexData, indexSize, elements);
		}

		void Initialize(void* vertexData, unsigned int vertexSize, void* indexData, unsigned int indexSize, std::vector<VGFX::VertexElement::IElement*> elements)
		{
			m_VertexArray.Create();

			m_Vertices = std::make_unique<VertexBuffer>(vertexData, vertexSize);
			m_Indices = std::make_unique<IndexBuffer>(indexData, indexSize);

			m_VertexArray.PushElement(elements);
			m_VertexArray.Unbind();
		}

		void Render() override;
		void Bind() const;
		virtual void Draw();
		void UnBind() const;

		VertexBuffer* GetVertexBuffer() const { return m_Vertices.get(); }
		IndexBuffer* GetIndexBuffer() const { return m_Indices.get(); }

		unsigned int VAO;
	private:
		std::unique_ptr<VertexBuffer> m_Vertices;
		std::unique_ptr<IndexBuffer> m_Indices;
		VertexArray m_VertexArray;
	};

}
