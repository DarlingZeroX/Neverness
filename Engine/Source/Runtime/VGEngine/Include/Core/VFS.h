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
#include "CoreTypes.h"
#include "../EngineConfig.h"
#include <HFileSystem/Include/VFS/VFS.h>

namespace VisionGal
{ 
	struct VG_ENGINE_API VFS
	{
		using DataRef = Ref<std::vector<uint8_t>>;

		/// <summary>
		/// 获取虚拟文件系统相对路径
		/// </summary>
		/// <param name="relativePath">虚拟文件系统路径</param>
		/// <param name="absolutePath">绝对路径</param>
		/// <returns>绝对路径转换后的虚拟文件系统的路径</returns>
		static String GetRelativePathVFS(const String& relativePath, const String& absolutePath);

		/// <summary>
		/// 获取相对资源目录的虚拟文件系统路径
		/// </summary>
		/// <param name="absolutePath">绝对路径</param>
		/// <returns>绝对路径转换后的资源目录虚拟文件系统的路径</returns>
		static String GetResourcePathVFS(const String& absolutePath);

		/// <summary>
		/// 获取虚拟文件系统
		/// </summary>
		/// <returns>返回虚拟文件系统引用</returns>
		static vfspp::VirtualFileSystemPtr& GetInstance();

		/// <summary>
		/// 从虚拟文件系统读取二进制文件
		/// </summary>
		/// <param name="path">虚拟文件系统路径</param>
		/// <param name="callback">读取成功后的二进制文件数据回调</param>
		/// <returns>如果读取错误返回-1,其他结果由用户设置</returns>
		//static int ReadFileFromVFS(const String& path, std::function<int(uint8_t*, uint64_t)> callback);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="path"></param>
		/// <param name="callback"></param>
		/// <returns></returns>
		static int SafeReadFileFromVFS(const String& path, std::function<int(const DataRef&)> callback);

		/// <summary>
		/// 从虚拟文件系统读取文本文件
		/// </summary>
		/// <param name="path">虚拟文件系统路径</param>
		/// <param name="callback">读取成功后的文本文件数据回调</param>
		/// <returns>如果读取错误返回-1, 其他结果由用户设置</returns>
		static bool ReadTextFromFile(const String& path, String& text);

		/// <summary>
		/// 保存文本文件到虚拟文件系统
		/// </summary>
		/// <param name="path">虚拟文件系统路径</param>
		/// <param name="str">文本数据</param>
		/// <returns>保存成功返回true, 失败返回false</returns>
		static bool WriteTextToFile(const String& path, const String& str);

		static bool MountPackageFileSystem(const String& alias, const String& pakPath, const String& backAsbPath);
	};

	struct VG_ENGINE_API IStringStreamVFS
	{
		~IStringStreamVFS();

		bool Open(const String& path);
		bool IsOpen() const;
		void Close();

		std::istringstream& GetStream();
	private:
		vfspp::IFilePtr m_FilePtr;
		std::istringstream m_IStringStream;
		bool m_IsOpen = false;
	};

}
