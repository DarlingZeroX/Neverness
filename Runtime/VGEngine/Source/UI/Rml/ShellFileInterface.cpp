/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "UI/Rml/ShellFileInterface.h"
#include <stdio.h>
#include "Core/VFS.h"

ShellFileInterface::ShellFileInterface(const Rml::String& root) : root(root) {}

ShellFileInterface::~ShellFileInterface() {}

Rml::FileHandle ShellFileInterface::Open(const Rml::String& path)
{
	// Attempt to open the file relative to the application's root.
	FILE* fp = fopen((root + path).c_str(), "rb");
	if (fp != nullptr)
		return (Rml::FileHandle)fp;

	// Attempt to open the file relative to the current working directory.
	fp = fopen(path.c_str(), "rb");
	return (Rml::FileHandle)fp;
}

void ShellFileInterface::Close(Rml::FileHandle file)
{
	fclose((FILE*)file);
}

size_t ShellFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
{
	return fread(buffer, 1, size, (FILE*)file);
}

bool ShellFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
	return fseek((FILE*)file, offset, origin) == 0;
}

size_t ShellFileInterface::Tell(Rml::FileHandle file)
{
	return ftell((FILE*)file);
}

UIFileInterfaceVFS::UIFileInterfaceVFS()
{
}

UIFileInterfaceVFS::~UIFileInterfaceVFS()
{
}

Rml::FileHandle UIFileInterfaceVFS::Open(const Rml::String& path)
{
	auto file = VisionGal::VFS::GetInstance()->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read);

	if (!file)
		return 0;

	if (!file->IsOpened())
		return 0;

	auto handle = GetNewFileHandle();
	m_FilPtrMap[handle] = file;

	return handle;
}

void UIFileInterfaceVFS::Close(Rml::FileHandle file)
{
	auto result = m_FilPtrMap.find(file);
	if (result != m_FilPtrMap.end())
	{
		result->second->Close();
	}
}

size_t UIFileInterfaceVFS::Read(void* buffer, size_t size, Rml::FileHandle file)
{
	auto result = m_FilPtrMap.find(file);
	if (result != m_FilPtrMap.end())
	{
		return result->second->Read(static_cast<uint8_t*>(buffer), size);
	}

	return 0;
}

bool UIFileInterfaceVFS::Seek(Rml::FileHandle file, long offset, int origin)
{
	auto result = m_FilPtrMap.find(file);
	if (result != m_FilPtrMap.end())
	{
		vfspp::IFile::Origin vfsOrigin;
		switch (origin) {
		case SEEK_SET:
			vfsOrigin = vfspp::IFile::Origin::Begin;
			break;
		case SEEK_CUR:
			vfsOrigin = vfspp::IFile::Origin::Set; //  这里注意：请确认Set是否代表 Current
			break;
		case SEEK_END:
			vfsOrigin = vfspp::IFile::Origin::End;
			break;
		default:
			return false; // 不支持
		}

		// 调用你的 Seek
		result->second->Seek(static_cast<uint64_t>(offset), vfsOrigin);
		return true;
	}

	return false;
}

size_t UIFileInterfaceVFS::Tell(Rml::FileHandle file)
{
	auto result = m_FilPtrMap.find(file);
	if (result != m_FilPtrMap.end())
	{
		return result->second->Tell();
	}

	return 0;
}

Rml::FileHandle UIFileInterfaceVFS::GetNewFileHandle()
{
	while (m_FilPtrMap.find(m_FileHandleIndex) != m_FilPtrMap.end())
	{
		m_FileHandleIndex++;
	}

	return m_FileHandleIndex;
}
