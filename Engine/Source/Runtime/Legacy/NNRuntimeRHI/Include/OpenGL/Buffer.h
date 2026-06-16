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
#include "Interface.h"

namespace NN::Runtime::OpenGL {

	class VG_RHI_API BufferGL
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

}
