#include "Core/Core.h"
#include <HCore/Include/System/HFileSystem.h>
#include <HCore/Include/VFS/VFS.h>

namespace VisionGal
{
	struct CoreImp
	{
		CoreImp()
		{
			VFS = CreateRef<vfspp::VirtualFileSystem>();
		}

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
		vfspp::VirtualFileSystemPtr VFS;
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
	};

	void Core::Initialize()
	{
		CoreImp::Get()->Initialize();
	}

	float Core::GetCurrentTime()
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = now - CoreImp::Get()->StartTime;
		return duration.count();
	}

	void Core::SetFileInterface(FileInterface* file_interface)
	{
		CoreImp::Get()->FileInterface = file_interface;
	}

	FileInterface* Core::GetFileInterface()
	{
		return CoreImp::Get()->FileInterface;
	}

	std::string Core::GetDefaultSpriteTexturePath()
	{
		return GetEngineResourcePathVFS() + "textures/white.png";
	}

	std::string Core::GetAssetsPathVFS()
	{
		return "/assets/";
	}

	std::string Core::GetProjectIntermediatePathVFS()
	{
		return "/projectIntermediate/";
	}

	std::string Core::GetProjectSettingsPathVFS()
	{
		return "/projectSettings/";
	}

	std::string Core::GetEngineResourcePathVFS()
	{
		return "/engine/";
	}

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#else // Linux / Unix
#include <unistd.h>
#endif

	std::filesystem::path Core::GetExecutableDirectory()
	{
#ifdef _WIN32
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		return std::filesystem::path(path).parent_path();
#elif __APPLE__
		char path[1024];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0) {
			return std::filesystem::path(path).parent_path();
		}
		return {};
#else // Linux
		char path[1024];
		ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
		if (count != -1) {
			path[count] = '\0';
			return std::filesystem::path(path).parent_path();
		}
		return {};
#endif
	}
}
