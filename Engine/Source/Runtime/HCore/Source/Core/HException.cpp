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

#include "pch.h"
#include "HException.h"
#include <sstream>

namespace Horizon
{
	EngineException::EngineException(int line, const char* file) noexcept
		:
		line(line),
		file(file)
	{}

	const char* EngineException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetWindowType() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* EngineException::GetWindowType() const noexcept
	{
		return "Exception";
	}

	int EngineException::GetLine() const noexcept
	{
		return line;
	}

	const std::string& EngineException::GetFile() const noexcept
	{
		return file;
	}

	std::string EngineException::GetOriginString() const noexcept
	{
		std::ostringstream oss;
		oss << "[File] " << file << std::endl
			<< "[Line] " << line;
		return oss.str();
	}
}