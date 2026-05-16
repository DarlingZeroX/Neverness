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

#include "FileInterface.h"
#include <RmlUi/Core/Log.h>

namespace NN::Runtime
{
	FileInterface::FileInterface()
	{
	}

	FileInterface::~FileInterface()
	{
	}

	size_t FileInterface::Length(FileHandle file)
	{
		size_t current_position = Tell(file);
		Seek(file, 0, SEEK_END);
		size_t length = Tell(file);
		Seek(file, (long)current_position, SEEK_SET);
		return length;
	}

	bool FileInterface::LoadFile(const std::string& path, std::string& out_data)
	{
		FileHandle handle = Open(path);
		if (!handle)
			return false;

		const size_t length = Length(handle);

		out_data = String(length, 0);

		const size_t read_length = Read(&out_data[0], length, handle);

		if (length != read_length)
		{
			H_LOG_WARN("Could only read %zu of %zu bytes from file %s", read_length, length, path.c_str());
		}

		Close(handle);

		return true;
	}
}