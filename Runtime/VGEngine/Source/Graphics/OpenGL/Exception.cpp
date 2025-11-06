#include "Graphics/OpenGL/Exception.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	void Exception::ClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	};

	bool Exception::CheckError(const char* function, const char* file, int line)
	{
		while (GLenum error = glGetError())
		{
			std::cout << "[OpenGL Error] (" << error << ")" << function << " " << file << ":" << line << std::endl;
			return true;
		}
		return false;
	};

	const char* Exception::what() const noexcept
	{
		std::ostringstream oss;

		oss << GetWindowType() << std::endl
			<< "\n[OpenGL Error]\n" << GetErrorInfo() << std::endl << std::endl;
		oss << GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	 
	std::string InitException::GetErrorInfo() const noexcept
	{
		std::ostringstream errorMsg;
		while (GLenum error = glGetError())
		{
			errorMsg << "(" << error << ") ";
		}
		return errorMsg.str();
	}

VISIONGAL_OPENGL_NAMESPACE_END


