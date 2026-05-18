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

#include "Rml/ShellFileInterface.h"
#include "NNRuntimeVFS/Include/VFSService.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"

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
	std::filesystem::path fspath = path;

	Rml::String processPath;
	PathType type = ProcessPath(path, processPath);

	//auto file = NN::Runtime::VFS::VFSService::GetInstance()->OpenFile(NN::Runtime::VFS::FileInfo(path), NN::Runtime::VFS::IFile::FileMode::Read);
	//
	//if (!file)
	//	return 0;
	//
	//if (!file->IsOpened())
	//	return 0;

	NN::Ref<FilePtr> filePtr = NN::MakeRef<FilePtr>();
	bool result = filePtr->Open(type, processPath);

	if (result == false)
		return 0;

	auto handle = GetNewFileHandle();
	m_FilPtrMap[handle] = filePtr;

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
		return result->second->Seek(static_cast<uint64_t>(offset), origin);
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

bool UIFileInterfaceVFS::FilePtr::Open(PathType pathType, const Rml::String& path)
{
	type = pathType;
	filePath = path;

	if (type == PathType::VFS)
	{
		fpVFS = NN::Runtime::VFS::VFSService::GetInstance()->OpenFile(NN::Runtime::VFS::FileInfo(path), NN::Runtime::VFS::IFile::FileMode::Read);

		if (!fpVFS)
			return false;

		if (!fpVFS->IsOpened())
			return false;

		// 事件
		NN::Runtime::UISystemEvent evt;
		evt.EventType = NN::Runtime::UISystemEventType::UIFileOpen;
		evt.UIFilePath = filePath;
		NN::Runtime::EngineEventBus::Get().OnUISystemEvent.Invoke(evt);

		return true;
	}

	if (type == PathType::FS)
	{
		// Attempt to open the file relative to the application's root.
		fp = fopen(path.c_str(), "rb");
		if (fp == nullptr)
			return false;

		return true;
	}

	return false;
}

void UIFileInterfaceVFS::FilePtr::Close()
{
	if (type == PathType::VFS)
	{
		fpVFS->Close();

		// 事件
		NN::Runtime::UISystemEvent evt;
		evt.EventType = NN::Runtime::UISystemEventType::UIFileClose;
		evt.UIFilePath = filePath;
		NN::Runtime::EngineEventBus::Get().OnUISystemEvent.Invoke(evt);
	}

	if (type == PathType::FS)
	{
		fclose(fp);
	}
}

size_t UIFileInterfaceVFS::FilePtr::Read(void* buffer, size_t size)
{
	if (type == PathType::VFS)
	{
		return fpVFS->Read(static_cast<uint8_t*>(buffer), size);
	}

	if (type == PathType::FS)
	{
		return fread(buffer, 1, size, fp);
	}
}

bool UIFileInterfaceVFS::FilePtr::Seek(long offset, int origin)
{
	if (type == PathType::VFS)
	{
		NN::Runtime::VFS::IFile::Origin vfsOrigin;
		switch (origin) {
		case SEEK_SET:
			vfsOrigin = NN::Runtime::VFS::IFile::Origin::Begin;
			break;
		case SEEK_CUR:
			vfsOrigin = NN::Runtime::VFS::IFile::Origin::Set; //  这里注意：请确认Set是否代表 Current
			break;
		case SEEK_END:
			vfsOrigin = NN::Runtime::VFS::IFile::Origin::End;
			break;
		default:
			return false; // 不支持
		}

		// 调用你的 Seek
		fpVFS->Seek(static_cast<uint64_t>(offset), vfsOrigin);
		return true;
	}

	if (type == PathType::FS)
	{
		return fseek(fp, offset, origin) == 0;
	}

	return false;
}

size_t UIFileInterfaceVFS::FilePtr::Tell()
{
	if (type == PathType::VFS)
	{
		return fpVFS->Tell();
	}

	if (type == PathType::FS)
	{
		return ftell(fp);
	}

	return 0;
}

UIFileInterfaceVFS::PathType UIFileInterfaceVFS::ProcessPath(const Rml::String& path, Rml::String& outPath)
{
	size_t absPos = path.find("[system]");
	if (absPos != std::string::npos)
	{
		outPath = path.substr(absPos + 8); // 8 是 "[system]" 的长度
		return PathType::FS;
	}

	size_t rootPos = path.find("[root]");
	if (rootPos != std::string::npos)
	{
		outPath = path.substr(rootPos + 6); // 6 是 "[root]" 的长度
		return PathType::VFS;
	}

	outPath = path;
	return PathType::VFS;
}

Rml::FileHandle UIFileInterfaceVFS::GetNewFileHandle()
{
	while (m_FilPtrMap.find(m_FileHandleIndex) != m_FilPtrMap.end())
	{
		m_FileHandleIndex++;
	}

	return m_FileHandleIndex;
}
