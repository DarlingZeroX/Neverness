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
#include "NNCore/Interface/HCoreTypes.h"

namespace NN::Core
{
	// Specifies how the operating system should open a file.
	enum class FileCreationMode : uint32
	{
		// Create a new file. If the file already exists, the call will fail.
		CreateNew = 1,

		// Create a new file. If the file already exists, it will be overwritten and truncated to 0 size.
		Create = 2,

		// Specifies that the operating system should open an existing file. If the file
		// doesn't exist the call will fail.
		Open = 3,

		// Open a file if it exists; otherwise, a new file will be created.
		OpenOrCreate = 4,

		// Open existing file, truncate it's size to 0.
		Truncate = 5,

		// Opens the file if it exists and seeks to the end of the file, or creates a new file.
		Append = 6,
	};

	BITFLAG_ENUM_CLASS_HELPER(FileCreationMode);

	enum class FileAccessMode 
	{
		// Chose the access mode automatically based on creation mode
		Default = -1,

		// Self-explanatory
		Read = 1,

		// Self-explanatory
		Write = 2,

		// Self-explanatory
		ReadWrite = 3,
	};

	BITFLAG_ENUM_CLASS_HELPER(FileAccessMode);

	struct FileShareMode
	{
		enum Enum
		{
			// Chose the mode automatically based on creation mode
			Default = -1,

			// Don't share.
			// Any request to open the file (by this process or another process) will fail until the
			// file is closed.
			None = 0,

			// Share only for read. Subsequent opening of the file for reading will be allowed.
			Read = 1,

			// Share only for write. Allows subsequent opening of the file for writing.
			Write = 2,

			// Share for read and write. Subsequent opening of the file for reading or writing will
			// be allowed.
			ReadWrite = 3,

			// Share for delete. Subsequent deleting of a file will be allowed.
			Delete = 4,
		};
	};
}
