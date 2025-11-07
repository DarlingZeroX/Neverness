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
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Types.h>
#include "../../Core/VFS.h"

/**
    RmlUi file interface for the shell examples.
    @author Lloyd Weehuizen
 */

class ShellFileInterface : public Rml::FileInterface {
public:
	ShellFileInterface(const Rml::String& root);
	virtual ~ShellFileInterface();

	/// Opens a file.
	Rml::FileHandle Open(const Rml::String& path) override;

	/// Closes a previously opened file.
	void Close(Rml::FileHandle file) override;

	/// Reads data from a previously opened file.
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;

	/// Seeks to a point in a previously opened file.
	bool Seek(Rml::FileHandle file, long offset, int origin) override;

	/// Returns the current position of the file pointer.
	size_t Tell(Rml::FileHandle file) override;

private:
	Rml::String root;
};

class UIFileInterfaceVFS: public Rml::FileInterface{
public:
	enum class PathType
	{
		FS,
		VFS
	};

	UIFileInterfaceVFS();
	~UIFileInterfaceVFS() override;

	/// Opens a file.
	Rml::FileHandle Open(const Rml::String& path) override;

	/// Closes a previously opened file.
	void Close(Rml::FileHandle file) override;

	/// Reads data from a previously opened file.
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;

	/// Seeks to a point in a previously opened file.
	bool Seek(Rml::FileHandle file, long offset, int origin) override;

	/// Returns the current position of the file pointer.
	size_t Tell(Rml::FileHandle file) override;

private:
	struct FilePtr
	{
		PathType type = PathType::VFS;
		vfspp::IFilePtr fpVFS = nullptr;
		FILE* fp = nullptr;

		/// Opens a file.
		bool Open(PathType pathType, const Rml::String& path);

		/// Closes a previously opened file.
		void Close();

		/// Reads data from a previously opened file.
		size_t Read(void* buffer, size_t size);

		/// Seeks to a point in a previously opened file.
		bool Seek(long offset, int origin);

		/// Returns the current position of the file pointer.
		size_t Tell();
	};

	static PathType ProcessPath(const Rml::String& path, Rml::String& outPath);

	Rml::FileHandle GetNewFileHandle();

	Rml::String root;

	std::unordered_map<Rml::FileHandle, Ref<FilePtr>> m_FilPtrMap;
	Rml::FileHandle m_FileHandleIndex = 1;
};
