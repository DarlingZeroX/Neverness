// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../CoreModuleDefinitions.h"
#include <exception>
#include <string>

namespace Horizon
{
	class CORE_MODULE_API EngineException : public std::exception
	{
	public:
		EngineException(int line, const char* file) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetWindowType() const noexcept;
		int GetLine() const noexcept;
		const std::string& GetFile() const noexcept;
		std::string GetOriginString() const noexcept;
	private:
		int line;
		std::string file;
	protected:
		mutable std::string whatBuffer;
	};
}
