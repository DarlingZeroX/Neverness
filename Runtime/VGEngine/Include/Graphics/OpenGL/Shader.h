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
#include "../Interface/Shader.h"
#include "OpenGL.h"
#include <string>

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	enum ShaderType
	{
		VERTEX_SHADER = GL_VERTEX_SHADER,
		FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
		GEOMETRY_SHADER = GL_GEOMETRY_SHADER
	};

	class Shader: public VGFX::IShader
	{
	public:
		Shader() = default;
		Shader(unsigned int type, const std::string& source);
		Shader(Shader&) = delete;
		Shader& operator=(Shader&) = delete;
		~Shader() override;
	public: 
		bool Exist() const;
		std::string GetErrorMsg() const;
		unsigned int GetRendererID();

		unsigned int GetResourceHandle() override;
		std::string GetType() override;
	private:
		bool CreateShaderInline(unsigned int type, const std::string& source);

		void Clear();
		unsigned int CreateGLShader(GLenum type);
		void GetShaderiv(GLenum pname, GLint* param);
		void ShaderSource(const char** src);
		void CompileShader();
		void DeleteShader();
		void GetShaderInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog);

		std::string m_ErrorMsg;
		unsigned int m_RendererID = 0;
	};

	Ref<Shader> CreateShader(unsigned int type, const std::string& source);

VISIONGAL_OPENGL_NAMESPACE_END
