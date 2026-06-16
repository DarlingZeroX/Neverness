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

#include "VFSService.h"
#include <NNFileSystem/Interface/HFileSystem.h>
#include <filesystem>
#include <unordered_map>
#include <utility>

#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "VFS/NativeFileSystem.h"
#include "VFS/ZipFileSystem.h"
#include "VFS/MemoryFileSystem.h"

namespace NN::Runtime::VFS
{
	struct VFSImp
	{
		VFSImp()
		{
			VFS = NN::MakeRef<VirtualFileSystem>();
		}

		static VFSImp* Get()
		{
			static VFSImp imp;
			return &imp;
		}

		VirtualFileSystemPtr VFS;
	};

	std::string VFSService::GetRelativePathVFS(const std::string& relativePath, const std::string& absolutePath)
	{
		auto path = NN::Core::HFileSystem::ToUnixPath(absolutePath);

		auto rPath = GetInstance()->AbsolutePath(relativePath);
		auto relative = std::filesystem::path(path).lexically_relative(rPath);

		if (relative == ".")
			return relativePath;

		return relativePath + relative.string();
	}

	std::string VFSService::GetAbsolutePath(const std::string& relativePath)
	{
		return GetInstance()->AbsolutePath(relativePath);
	}

	VirtualFileSystemPtr& VFSService::GetInstance()
	{
		return VFSImp::Get()->VFS;
	}

	bool VFSService::MountFileSystem(const std::string& alias, IFileSystemPtr fs)
	{
		if (!fs)
			return false;

		fs->Initialize();
		GetInstance()->AddFileSystem(alias, std::move(fs));
		return true;
	}

	int VFSService::SafeReadFileFromVFS(const std::string& path, std::function<int(const DataRef&)> callback)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(FileInfo(path), IFile::FileMode::Read))
		{
			if (file->IsOpened())
			{
				NN::Ref<std::vector<uint8_t>> fileData = NN::MakeRef<std::vector<uint8_t>>();
				fileData->resize(file->Size());

				file->Read(fileData->data(), fileData->size());
				file->Close();

				return callback(fileData);
			}
		}

		return -1;
	}

	bool VFSService::ReadTextFromFile(const std::string& path, std::string& text)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(FileInfo(path), IFile::FileMode::Read))
		{
			if (file->IsOpened())
			{
				std::string content;
				content.resize(file->Size());
				file->Read(reinterpret_cast<uint8_t*>(content.data()), content.size());
				file->Close();

				text = content;
				return true;
			}
		}

		return false;
	}

	bool VFSService::WriteTextToFile(const std::string& path, const std::string& str)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(FileInfo(path), IFile::FileMode::Write))
		{
			if (file->IsOpened())
			{
				std::string& data = const_cast<std::string&>(str);
				file->Write(reinterpret_cast<uint8_t*>(data.data()), data.size());
				file->Close();

				H_LOG_INFO("Text written to file: %s", path.c_str());
				return true;
			}
		}

		H_LOG_WARN("Failed to write text to file: %s", path.c_str());
		return false;
	}

	bool VFSService::WriteBufferToFile(const std::string& path, uint8_t* buffer, uint64_t size)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(FileInfo(path), IFile::FileMode::Write))
		{
			if (file->IsOpened())
			{
				file->Write(buffer, size);
				file->Close();

				return true;
			}
		}

		return false;
	}

	bool VFSService::RebuildNativeFileSystemFiles(const std::string& path)
	{		// 需要刷新虚拟文件系统
		auto fsList = GetInstance()->GetFilesystems(path);
		if (fsList.size() == 1)
		{
			auto fs = fsList.begin()->get();
			NativeFileSystem* nfs = dynamic_cast<NativeFileSystem*>(fs);
			nfs->RebuildFileList();

			return true;
		}

		return false;
	}

	// ── handle 注册表（handle → {alias, IFileSystemPtr}） ──

	namespace
	{
		/** 文件系统类型枚举，与 NNVfsFileSystemType 对齐 */
		enum class FSType : uint32_t
		{
			Native = 0,
			Zip    = 1,
			Memory = 2,
		};

		std::unordered_map<uint64_t, std::pair<std::string, IFileSystemPtr>>& GetFSRegistry()
		{
			static std::unordered_map<uint64_t, std::pair<std::string, IFileSystemPtr>> registry;
			return registry;
		}

		uint64_t& GetNextHandle()
		{
			static uint64_t next = 1;
			return next;
		}
	}

	uint64_t VFSService::AddFileSystem(const std::string& alias, uint32_t type, const std::string& path)
	{
		IFileSystemPtr fs;

		switch (static_cast<FSType>(type))
		{
		case FSType::Native:
			fs = std::make_shared<NativeFileSystem>(path);
			break;
		case FSType::Zip:
			fs = std::make_shared<ZipFileSystem>(path);
			break;
		case FSType::Memory:
			fs = std::make_shared<MemoryFileSystem>();
			break;
		default:
			return 0;
		}

		if (!fs)
			return 0;

		fs->Initialize();
		GetInstance()->AddFileSystem(alias, fs);

		uint64_t handle = GetNextHandle()++;
		GetFSRegistry()[handle] = { alias, std::move(fs) };
		return handle;
	}

	bool VFSService::RemoveFileSystem(uint64_t handle)
	{
		auto& registry = GetFSRegistry();
		auto it = registry.find(handle);
		if (it == registry.end())
			return false;

		GetInstance()->RemoveFileSystem(it->second.first, it->second.second);
		registry.erase(it);
		return true;
	}

	bool VFSService::HasFileSystem(uint64_t handle)
	{
		auto& registry = GetFSRegistry();
		auto it = registry.find(handle);
		if (it == registry.end())
			return false;

		return GetInstance()->HasFileSystem(it->second.first, it->second.second);
	}

	void VFSService::UnregisterAlias(const std::string& alias)
	{
		GetInstance()->UnregisterAlias(alias);

		// 清理注册表中该 alias 的所有条目
		auto& registry = GetFSRegistry();
		for (auto it = registry.begin(); it != registry.end(); )
		{
			if (it->second.first == alias)
				it = registry.erase(it);
			else
				++it;
		}
	}

	bool VFSService::IsAliasRegistered(const std::string& alias)
	{
		return GetInstance()->IsAliasRegistered(alias);
	}

	IStringStreamVFS::~IStringStreamVFS()
	{
		Close();
	}

	void IStringStreamVFS::Close()
	{
		if (m_FilePtr)
			m_FilePtr->Close();
	}

	bool IStringStreamVFS::Open(const std::string& path)
	{
		m_FilePtr = VFSService::GetInstance()->OpenFile(FileInfo(path), IFile::FileMode::Read);
		if (m_FilePtr == nullptr)
			return false;

		if (!m_FilePtr->IsOpened())
			return false;

		uint64_t fileSize = m_FilePtr->Size();
		if (fileSize == 0) {
			return false;
		}

		std::ostringstream outputStream;
		m_FilePtr->Read(outputStream, fileSize, 4096);

		std::string serializedData = outputStream.str();
		std::istringstream inputStream(serializedData);
		m_IStringStream = std::move(inputStream);

		m_IsOpen = true;
		return true;
	}

	bool IStringStreamVFS::IsOpen() const
	{
		return m_IsOpen;
	}

	std::istringstream& IStringStreamVFS::GetStream()
	{
		return m_IStringStream;
	}
}
