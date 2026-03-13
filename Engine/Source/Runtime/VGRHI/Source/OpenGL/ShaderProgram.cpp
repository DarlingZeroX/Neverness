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

#include "OpenGL/ShaderProgram.h"
#include "OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	void ShaderProgram::CreateProgram()
	{
		GL_THROW_INFO(m_RendererID = glCreateProgram());
	}

	void ShaderProgram::LinkProgram() const
	{
		GL_THROW_INFO(glLinkProgram(m_RendererID));
	}

	void ShaderProgram::ValidateProgram() const
	{
		GL_THROW_INFO(glValidateProgram(m_RendererID));
	}

	void ShaderProgram::UseProgram() const
	{
		GL_THROW_INFO(glUseProgram(m_RendererID));
	}

	void ShaderProgram::AttachShader(VGFX::IShader* shader) const
	{
		GL_THROW_INFO(glAttachShader(m_RendererID, shader->GetResourceHandle()));
	} 

	void ShaderProgram::DeleteProgram()
	{
		if (m_RendererID != NULL)
		{
			glDeleteProgram(m_RendererID);
			m_RendererID = NULL;
		}
	}

	void ShaderProgram::Clear()
	{
		DeleteProgram();
	}

	void ShaderProgram::GetProgramiv(GLenum pname, GLint* param)
	{
		GL_THROW_INFO(glGetProgramiv(m_RendererID, pname, param));
	}

	void ShaderProgram::GetProgramInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)
	{
		GL_THROW_INFO(glGetProgramInfoLog(m_RendererID, bufSize, length, infoLog));
	}

	ShaderProgram::ShaderProgram(const std::vector<Shader*>& shaders)
	{
		CreateImp(shaders);
	}

	Ref<ShaderProgram> ShaderProgram::Create(const std::vector<Shader*>& shaders)
	{
		auto sp = CreateRef<ShaderProgram>();
		sp->CreateImp(shaders);

		return sp;
	}

	Ref<ShaderProgram> ShaderProgram::Create(const std::vector<VGFX::IShader*>& shaders)
	{
		auto sp = CreateRef<ShaderProgram>();
		sp->CreateImp(shaders);

		return sp;
	}

	bool ShaderProgram::CreateImp(const std::vector<VGFX::IShader*>& shaders)
	{
		CreateProgram();

		//attach shaders
		for (auto& shader : shaders)
		{
			AttachShader(shader);
		}

		LinkProgram();

		//check link status
		int result;
		GetProgramiv(GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			GetProgramiv(GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)_malloca(length * sizeof(char));

			//log error msg
			GetProgramInfoLog(length, &length, message);
			m_ErrorMsg = message;

			std::cout << message << std::endl;

			Clear();
			return false;
		}

		ValidateProgram();

		Bind();
		return true;
	}

	bool ShaderProgram::CreateImp(const std::vector<Shader*>& shaders)
	{
		CreateProgram();

		//attach shaders
		for (auto& shader : shaders)
		{
			AttachShader(shader);
		}

		LinkProgram();

		//check link status
		int result;
		GetProgramiv(GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			GetProgramiv(GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)_malloca(length * sizeof(char));

			//log error msg
			GetProgramInfoLog(length, &length, message);
			m_ErrorMsg = message;

			std::cout << message << std::endl;

			Clear();
			return false;
		}

		ValidateProgram();

		Bind();
		return true;
	}

	std::string ShaderProgram::GetErrorMsg() const
	{
		return m_ErrorMsg;
	}

	void ShaderProgram::UseProgram()
	{
		Bind();
	}

	bool ShaderProgram::Exist() const
	{
		return glIsProgram(m_RendererID);
	}

	void ShaderProgram::DetachShader(GLuint shader)
	{
		GL_THROW_INFO(glDetachShader(m_RendererID, shader));
	}

	void ShaderProgram::Bind() const
	{
		GL_THROW_INFO(glUseProgram(m_RendererID));
	}

	void ShaderProgram::Unbind() const
	{
		GL_THROW_INFO(glUseProgram(0));
	}

	void ShaderProgram::SetInt(const std::string& name, int v)
	{
		GL_THROW_INFO(glUniform1i(GetUniformLocation(name), v));
	}

	void ShaderProgram::SetInt2(const std::string& name, Horizon::int2 v)
	{
		GL_THROW_INFO(glUniform2i(GetUniformLocation(name), v.x, v.y));
	}

	void ShaderProgram::SetInt3(const std::string& name, Horizon::int3 v)
	{
		GL_THROW_INFO(glUniform3i(GetUniformLocation(name), v.x, v.y, v.z));
	}

	void ShaderProgram::SetInt4(const std::string& name, Horizon::int4 v)
	{
		GL_THROW_INFO(glUniform4i(GetUniformLocation(name), v.x, v.y, v.z, v.w));
	}

	void ShaderProgram::SetBool(const std::string& name, bool v)
	{
		GL_THROW_INFO(glUniform1i(GetUniformLocation(name), (GLint)v));
	}

	void ShaderProgram::SetFloat(const std::string& name, float v)
	{
		GL_THROW_INFO(glUniform1f(GetUniformLocation(name), v));
	}

	void ShaderProgram::SetFloat2(const std::string& name, const Horizon::float2& v)

	{
		GL_THROW_INFO(glUniform2f(GetUniformLocation(name), v.x, v.y));
	}

	void ShaderProgram::SetFloat3(const std::string& name, const Horizon::float3& v3)
	{
		GL_THROW_INFO(glUniform3f(GetUniformLocation(name), v3.x, v3.y, v3.z));
	}

	void ShaderProgram::SetFloat4(const std::string& name, const Horizon::float4& v4)
	{
		GL_THROW_INFO(glUniform4f(GetUniformLocation(name), v4.x, v4.y, v4.z, v4.w));
	}

	void ShaderProgram::SetMatrix2(const std::string& name, const Horizon::matrix2x2& matrix)
	{
		GL_THROW_INFO(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
	}

	void ShaderProgram::SetMatrix3(const std::string& name, const Horizon::matrix3x3& matrix)
	{
		GL_THROW_INFO(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
	}

	void ShaderProgram::SetMatrix4(const std::string& name, const Horizon::matrix4x4& matrix)
	{
		GL_THROW_INFO(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
	}

	int ShaderProgram::GetUniformLocation(const std::string& name)
	{
		//search local cache
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];

		//query location
		GL_THROW_INFO(int location = glGetUniformLocation(m_RendererID, name.c_str()));
		if (location == -1)
		{
			std::cout << "Warning: uniform '" << name << " doesn't exist" << std::endl;
			return location;
		}

		//cache location
		m_UniformLocationCache[name] = location;
		return location;
	}

VISIONGAL_OPENGL_NAMESPACE_END


