#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	Shader::Shader(unsigned int type, const std::string& source)
	{
		CreateShaderInline(type, source);
	}

	Shader::~Shader()
	{
		Clear();
	}

	bool Shader::Exist() const
	{
		return glIsShader(m_RendererID);
	}

	bool Shader::CreateShaderInline(unsigned int type, const std::string& source)
	{
		H_ASSERT_NULL(m_RendererID);

		m_RendererID = CreateGLShader(type);
		if (m_RendererID == NULL)
		{
			return false;
		} 

		const char* src = source.c_str();
		ShaderSource(&src);
		CompileShader();

		int result;
		GetShaderiv(GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			GetShaderiv(GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)_malloca(length * sizeof(char));

			GetShaderInfoLog(length, &length, message);
			m_ErrorMsg = message;

			std::cout << message << std::endl;

			Clear();
			return false;
		}

		return true;
	}

	std::string Shader::GetErrorMsg() const
	{
		return m_ErrorMsg;
	}

	void Shader::Clear()
	{
		DeleteShader();
	}

	unsigned int Shader::CreateGLShader(GLenum type)
	{
		unsigned int id = glCreateShader(type);
		//GL_THROW_INFO(unsigned int id = glCreateShader(type);)
		return id;
	}

	void Shader::GetShaderiv(GLenum pname, GLint* param)
	{
		GL_THROW_INFO(glGetShaderiv(m_RendererID, pname, param));
	}

	void Shader::ShaderSource(const char** src)

	{
		GL_THROW_INFO(glShaderSource(m_RendererID, 1, src, nullptr));
	}

	void Shader::CompileShader()
	{
		GL_THROW_INFO(glCompileShader(m_RendererID));
	}

	void Shader::DeleteShader()
	{
		if (m_RendererID != NULL)
		{
			glDeleteShader(m_RendererID);
			m_RendererID = NULL;
		}
	}

	void Shader::GetShaderInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)
	{
		glGetShaderInfoLog(m_RendererID, bufSize, length, infoLog);
	}

	unsigned int Shader::GetRendererID()
	{
		return m_RendererID;
	}

	unsigned int Shader::GetResourceHandle()
	{
		return GetRendererID();
	}

	std::string Shader::GetType()
	{
		return "Unknown";
	}

	Ref<Shader> CreateShader(unsigned int type, const std::string& source)
	{
		Ref<Shader> shader = CreateRef<Shader>(type, source);

		return shader;
	}

VISIONGAL_OPENGL_NAMESPACE_END

