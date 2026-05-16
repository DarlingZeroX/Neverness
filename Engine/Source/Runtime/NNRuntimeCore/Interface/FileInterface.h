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

#include "../Include/Core/CoreTypes.h"

namespace VisionGal
{
	class FileInterface {
public:
	FileInterface();
	virtual ~FileInterface();

	/// Opens a file.
	/// @param path The path to the file to open.
	/// @return A valid file handle, or nullptr on failure
	virtual FileHandle Open(const std::string& path) = 0;
	/// Closes a previously opened file.
	/// @param file The file handle previously opened through Open().
	virtual void Close(FileHandle file) = 0;

	/// Reads data from a previously opened file.
	/// @param buffer The buffer to be read into.
	/// @param size The number of bytes to read into the buffer.
	/// @param file The handle of the file.
	/// @return The total number of bytes read into the buffer.
	virtual size_t Read(void* buffer, size_t size, FileHandle file) = 0;
	/// Seeks to a point in a previously opened file.
	/// @param file The handle of the file to seek.
	/// @param offset The number of bytes to seek.
	/// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from
	/// the current file position).
	/// @return True if the operation completed successfully, false otherwise.
	virtual bool Seek(FileHandle file, long offset, int origin) = 0;
	/// Returns the current position of the file pointer.
	/// @param file The handle of the file to be queried.
	/// @return The number of bytes from the origin of the file.
	virtual size_t Tell(FileHandle file) = 0;

	/// Returns the length of the file.
	/// The default implementation uses Seek & Tell.
	/// @param file The handle of the file to be queried.
	/// @return The length of the file in bytes.
	virtual size_t Length(FileHandle file);

	/// Load and return a file.
	/// @param path The path to the file to load.
	/// @param out_data The string contents of the file.
	/// @return True on success.
	virtual bool LoadFile(const std::string& path, std::string& out_data);
};
}