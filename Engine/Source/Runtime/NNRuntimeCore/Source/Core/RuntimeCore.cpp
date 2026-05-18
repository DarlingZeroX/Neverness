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

#include "Core/RuntimeCore.h"
#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNRuntimeVFS/Include/VFSService.h>

namespace NN::Runtime
{
	struct CoreImp
	{
		static CoreImp* Get()
		{
			static CoreImp imp;
			return &imp;
		}

		void Initialize()
		{
			StartTime = std::chrono::high_resolution_clock::now();
		}

		FileInterface* FileInterface;
		std::string ContentRootDirectory;
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
	};

	void RuntimeCore::Initialize()
	{
		CoreImp::Get()->Initialize();
	}

	float RuntimeCore::GetCurrentTime()
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = now - CoreImp::Get()->StartTime;
		return duration.count();
	}

	void RuntimeCore::SetFileInterface(FileInterface* file_interface)
	{
		CoreImp::Get()->FileInterface = file_interface;
	}

	FileInterface* RuntimeCore::GetFileInterface()
	{
		return CoreImp::Get()->FileInterface;
	}

	std::string RuntimeCore::GetDefaultSpriteTexturePath()
	{
		return GetEngineResourcePathVFS() + "textures/white.png";
	}

	std::string RuntimeCore::GetAssetsPathVFS()
	{
		return "/assets/";
	}

	std::string RuntimeCore::GetProjectIntermediatePathVFS()
	{
		return "/projectIntermediate/";
	}

	std::string RuntimeCore::GetProjectSettingsPathVFS()
	{
		return "/projectSettings/";
	}

	std::string RuntimeCore::GetEngineResourcePathVFS()
	{
		return "/engine/";
	}

	std::string RuntimeCore::GetResourcePathVFS(const std::string& absolutePath)
	{
		return VFS::VFSService::GetRelativePathVFS(GetAssetsPathVFS(), absolutePath);
	}

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#else // Linux / Unix
#include <unistd.h>
#endif

//	std::filesystem::path Core::GetExecutableDirectory()
//	{
//#ifdef _WIN32
//		char path[MAX_PATH];
//		GetModuleFileNameA(NULL, path, MAX_PATH);
//		return std::filesystem::path(path).parent_path();
//#elif __APPLE__
//		char path[1024];
//		uint32_t size = sizeof(path);
//		if (_NSGetExecutablePath(path, &size) == 0) {
//			return std::filesystem::path(path).parent_path();
//		}
//		return {};
//#else // Linux
//		char path[1024];
//		ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
//		if (count != -1) {
//			path[count] = '\0';
//			return std::filesystem::path(path).parent_path();
//		}
//		return {};
//#endif
//	}

	std::filesystem::path RuntimeCore::GetExecutableDirectory()
	{
#ifdef _WIN32
		wchar_t path[MAX_PATH];
		DWORD len = GetModuleFileNameW(NULL, path, MAX_PATH);
		if (len > 0 && len < MAX_PATH)
		{
			return std::filesystem::path(path).parent_path();
		}
		return {};

#elif __APPLE__
		char path[1024];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0) {
			return std::filesystem::path(path).parent_path();
		}
		return {};

#else // Linux
		char path[1024];
		ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
		if (count != -1) {
			path[count] = '\0';
			return std::filesystem::path(path).parent_path();
		}
		return {};
#endif
	}

}
