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

#include "OpenGL/Exception.h"

namespace NN::Runtime::OpenGL {

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

}


