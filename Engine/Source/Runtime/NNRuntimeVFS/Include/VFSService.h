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
#include <cstdint>
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

		// ── C# 可用的文件系统管理 API（handle 机制） ──

		/** 创建并挂载文件系统，返回不透明 handle（0 失败）。type: 0=Native, 1=Zip, 2=Memory。 */
		static uint64_t AddFileSystem(const std::string& alias, uint32_t type, const std::string& path);

		/** 根据 handle 精确移除指定文件系统。 */
		static bool RemoveFileSystem(uint64_t handle);

		/** 查询 handle 对应的文件系统是否仍挂载在 VFS 中。 */
		static bool HasFileSystem(uint64_t handle);

		/** 移除 alias 下全部文件系统。 */
		static void UnregisterAlias(const std::string& alias);

		/** 查询 alias 是否有任何文件系统已注册。 */
		static bool IsAliasRegistered(const std::string& alias);
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
