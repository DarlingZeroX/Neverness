#pragma once

#include "Interface.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class BufferGL
	{
	public:
		BufferGL() = default;
		BufferGL(BufferGL&) = delete;
		BufferGL& operator=(BufferGL&) = delete;
		BufferGL(BufferGL&& buf) noexcept;
		BufferGL& operator=(BufferGL&& buf) noexcept;
		~BufferGL();
	public:
		void Gen(int size = 1);
		void Bind(GLenum eTarget) const;
		void Unbind(GLenum eTarget) const;
		void Delete();

		void SetData(GLenum eTarget, GLsizeiptr size, const void* data, GLenum usage) const;
		void SetStaticData(GLenum eTarget, GLsizeiptr size, const void* data) const;
		void SetDynamicData(GLenum eTarget, GLsizeiptr size, const void* data) const;
		void SetStreamData(GLenum eTarget, GLsizeiptr size, const void* data) const;
		 
		GLuint GetRendererID() const noexcept;
	private:
		unsigned int m_Size = 0;
		GLuint m_RendererID = 0;
	};

VISIONGAL_OPENGL_NAMESPACE_END
