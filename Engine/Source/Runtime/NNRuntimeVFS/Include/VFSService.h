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

#include "../RuntimeVFSExport.h"
#include <NNCore/Interface/HConfig.h>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include "VFS/VFS.h"

namespace NN::Runtime::VFS
{
	struct NN_RUNTIME_VFS_API VFSService
	{
		using DataRef = NN::Ref<std::vector<uint8_t>>;

		static std::string GetRelativePathVFS(const std::string& relativePath, const std::string& absolutePath);

		static std::string GetAbsolutePath(const std::string& relativePath);

		static VirtualFileSystemPtr& GetInstance();

		static int SafeReadFileFromVFS(const std::string& path, std::function<int(const DataRef&)> callback);

		static bool ReadTextFromFile(const std::string& path, std::string& text);

		static bool WriteTextToFile(const std::string& path, const std::string& str);

		static bool WriteBufferToFile(const std::string& path, uint8_t* buffer, uint64_t size);

		static bool RebuildNativeFileSystemFiles(const std::string& path);

		static bool MountFileSystem(const std::string& alias, IFileSystemPtr fs);
	};

	struct NN_RUNTIME_VFS_API IStringStreamVFS
	{
		~IStringStreamVFS();

		bool Open(const std::string& path);
		bool IsOpen() const;
		void Close();

		std::istringstream& GetStream();

	private:
		IFilePtr m_FilePtr;
		std::istringstream m_IStringStream;
		bool m_IsOpen = false;
	};
}
