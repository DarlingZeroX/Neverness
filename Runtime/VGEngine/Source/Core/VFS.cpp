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

#include "Core/VFS.h"
#include "HCore/Include/System/HFileSystem.h"
#include "VGPackage/Include/PackageFileSystem.h"

namespace VisionGal
{
	struct VFSImp
	{
		VFSImp()
		{
			VFS = CreateRef<vfspp::VirtualFileSystem>();
		}

		static VFSImp* Get()
		{
			static VFSImp imp;
			return &imp;
		}

		vfspp::VirtualFileSystemPtr VFS;
	};

	String VFS::GetRelativePathVFS(const String& relativePath, const String& absolutePath)
	{
		//auto path = Horizon::HFileSystem::ToUnixPath(absolutePath);
		//
		//auto aPath = GetInstance()->AbsolutePath(relativePath);
		//auto relative = std::filesystem::relative(path, aPath).string();
		//
		//if (relative == ".")
		//	return relativePath;
		//
		//return relativePath + std::filesystem::relative(path, aPath).string();

		auto path = Horizon::HFileSystem::ToUnixPath(absolutePath);
		
		auto rPath = GetInstance()->AbsolutePath(relativePath);
		//auto relative = std::filesystem::relative(path, aPath).string();
		auto relative = std::filesystem::path(path).lexically_relative(rPath);

		if (relative == ".")
			return relativePath;

		return relativePath + relative.string();
	}

	String VFS::GetResourcePathVFS(const String& absolutePath)
	{
		return GetRelativePathVFS(Core::GetAssetsPathVFS(), absolutePath);
	}

	vfspp::VirtualFileSystemPtr& VFS::GetInstance()
	{
		return VFSImp::Get()->VFS;
	}

	int VFS::SafeReadFileFromVFS(const String& path, std::function<int(const DataRef&)> callback)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read))
		{
			if (file->IsOpened())
			{
				Ref<std::vector<uint8_t>> fileData = CreateRef<std::vector<uint8_t>>();
				fileData->resize(file->Size());

				//auto* fileData = new uint8_t[file->Size()];
				//auto dataSize = file->Size();
				file->Read(fileData->data(), fileData->size());
				file->Close();

				return callback(fileData);
			}
		}

		return -1;
	}

	//int VFS::ReadFileFromVFS(const String& path, std::function<int(uint8_t*, uint64_t)> callback)
	//{
	//	auto& vfs = GetInstance();
	//
	//	if (auto file = vfs->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read))
	//	{
	//		if (file->IsOpened())
	//		{
	//			auto* fileData = new uint8_t[file->Size()];
	//			auto dataSize = file->Size();
	//			file->Read(fileData, dataSize);
	//			file->Close();
	//
	//			return callback(fileData, dataSize);
	//		}
	//	}
	//
	//	return -1;
	//}

	bool VFS::ReadTextFromFile(const String& path, String& text)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read))
		{
			if (file->IsOpened())
			{
				// 读取文件内容到字符串
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

	bool VFS::WriteTextToFile(const String& path, const String& str)
	{
		auto& vfs = GetInstance();

		if (auto file = vfs->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Write))
		{
			if (file->IsOpened())
			{
				String& data = const_cast<String&>(str);
				file->Write(reinterpret_cast<uint8_t*>(data.data()), data.size());
				file->Close();

				return true;
			}
		}

		return false;
	}

	bool VFS::MountPackageFileSystem(const String& alias, const String& pakPath, const String& backAsbPath)
	{	auto& vfs = GetInstance();

		vfspp::IFileSystemPtr fsPtr;
		if (std::filesystem::exists(pakPath))
		{
			fsPtr = std::make_unique<vfspp::VGPackageFileSystem>(pakPath);
		}
		else
		{
			fsPtr = std::make_unique<vfspp::NativeFileSystem>(backAsbPath);

			if (std::filesystem::exists(backAsbPath) == false)
			{
				H_LOG_ERROR("无法挂载可用的文件系统: %s", pakPath.c_str());
				return false;
			}
		}

		fsPtr->Initialize();
		vfs->AddFileSystem(alias, std::move(fsPtr));

		return true;
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

	bool IStringStreamVFS::Open(const String& path)
	{
		// 1. 打开一个场景文件
		m_FilePtr = VFS::GetInstance()->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read);
		if (m_FilePtr == nullptr)
			return false;

		if (!m_FilePtr->IsOpened())
			return false;

		// 2. 确定文件大小（用于读取操作）
		uint64_t fileSize = m_FilePtr->Size();
		if (fileSize == 0) {
			H_LOG_ERROR("场景文件为空: %s", path.c_str());
			return false;
		}

		// 3. 创建输出流用于接收文件数据
		std::ostringstream outputStream;

		// 4. 使用vfspp的Read方法将文件内容读入内存流
		m_FilePtr->Read(outputStream, fileSize, 4096);

		// 5. 获取包含序列化数据的字符串
		std::string serializedData = outputStream.str();

		//std::cout << "Serialized Data" << serializedData << std::endl;

		// 创建输入流
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
