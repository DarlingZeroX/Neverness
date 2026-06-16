/**
 * @file VfsRuntimeApi.cpp
 * @brief **NNVfsAPI** Runtime 实现：转发至 **VFSService**，跨边界内存使用 **malloc/free**。
 *
 * 设计说明：
 * - C# / NativeAOT 仅通过函数表调用本 TU，不接触 C++ STL 类型。
 * - 读路径分配的内存在托管侧拷贝后必须调用 freeBuffer，避免 CRT 不匹配（禁止 new/delete 跨 DLL）。
 */

#include "VfsApiExport.h"

#include "VFSService.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/VfsAPI.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace
{

/** @brief 将 std::string 内容拷贝到 malloc 缓冲区（含 NUL 结尾）。 */
char* AllocateCString(const std::string& text)
{
	const std::size_t len = text.size();
	char* buffer = static_cast<char*>(std::malloc(len + 1));
	if (buffer == nullptr)
	{
		return nullptr;
	}
	if (len > 0)
	{
		std::memcpy(buffer, text.data(), len);
	}
	buffer[len] = '\0';
	return buffer;
}

/** @brief 将字节向量拷贝到 malloc 缓冲区。 */
std::uint8_t* AllocateBytes(const std::vector<std::uint8_t>& data, std::uint32_t& outSize)
{
	outSize = 0;
	if (data.empty())
	{
		std::uint8_t* empty = static_cast<std::uint8_t*>(std::malloc(1));
		if (empty == nullptr)
		{
			return nullptr;
		}
		outSize = 0;
		return empty;
	}

	const auto size = static_cast<std::uint32_t>(data.size());
	std::uint8_t* buffer = static_cast<std::uint8_t*>(std::malloc(size));
	if (buffer == nullptr)
	{
		return nullptr;
	}
	std::memcpy(buffer, data.data(), size);
	outSize = size;
	return buffer;
}

int NN_ENGINE_ABI_STDCALL VfsReadText(const char* pathUtf8, char** outText)
{
	if (pathUtf8 == nullptr || outText == nullptr)
	{
		return 0;
	}

	*outText = nullptr;

	std::string text;
	if (!NN::Runtime::VFS::VFSService::ReadTextFromFile(pathUtf8, text))
	{
		return 0;
	}

	char* buffer = AllocateCString(text);
	if (buffer == nullptr)
	{
		return 0;
	}

	*outText = buffer;
	return 1;
}

int NN_ENGINE_ABI_STDCALL VfsWriteText(const char* pathUtf8, const char* textUtf8)
{
	if (pathUtf8 == nullptr || textUtf8 == nullptr)
	{
		return 0;
	}

	return NN::Runtime::VFS::VFSService::WriteTextToFile(pathUtf8, textUtf8) ? 1 : 0;
}

int NN_ENGINE_ABI_STDCALL VfsReadBytes(const char* pathUtf8, std::uint8_t** outData, std::uint32_t* outSize)
{
	if (pathUtf8 == nullptr || outData == nullptr || outSize == nullptr)
	{
		return 0;
	}

	*outData = nullptr;
	*outSize = 0;

	std::vector<std::uint8_t> captured;
	const int readResult = NN::Runtime::VFS::VFSService::SafeReadFileFromVFS(
		pathUtf8,
		[&captured](const NN::Runtime::VFS::VFSService::DataRef& data) -> int {
			if (data == nullptr || data->empty())
			{
				return 0;
			}
			captured.assign(data->begin(), data->end());
			return 0;
		});

	if (readResult < 0)
	{
		return 0;
	}

	std::uint32_t size = 0;
	std::uint8_t* buffer = AllocateBytes(captured, size);
	if (buffer == nullptr && !captured.empty())
	{
		return 0;
	}

	*outData = buffer;
	*outSize = size;
	return 1;
}

void NN_ENGINE_ABI_STDCALL VfsFreeBuffer(void* buffer)
{
	if (buffer != nullptr)
	{
		std::free(buffer);
	}
}

int NN_ENGINE_ABI_STDCALL VfsGetRelativePath(
	const char* relativePathUtf8,
	const char* absolutePathUtf8,
	char** outPathUtf8)
{
	if (relativePathUtf8 == nullptr || absolutePathUtf8 == nullptr || outPathUtf8 == nullptr)
	{
		return 0;
	}

	*outPathUtf8 = nullptr;

	const std::string result = NN::Runtime::VFS::VFSService::GetRelativePathVFS(
		relativePathUtf8,
		absolutePathUtf8);

	char* buffer = AllocateCString(result);
	if (buffer == nullptr)
	{
		return 0;
	}

	*outPathUtf8 = buffer;
	return 1;
}

int NN_ENGINE_ABI_STDCALL VfsRebuildNativeFileSystemFiles(const char* pathUtf8)
{
	if (pathUtf8 == nullptr)
	{
		return 0;
	}

	return NN::Runtime::VFS::VFSService::RebuildNativeFileSystemFiles(pathUtf8) ? 1 : 0;
}

int NN_ENGINE_ABI_STDCALL VfsGetAbsolutePath(const char* relativePathUtf8, char** outAbsolutePathUtf8)
{
	if (relativePathUtf8 == nullptr || outAbsolutePathUtf8 == nullptr)
	{
		return 0;
	}

	*outAbsolutePathUtf8 = nullptr;

	const std::string result = NN::Runtime::VFS::VFSService::GetAbsolutePath(relativePathUtf8);

	char* buffer = AllocateCString(result);
	if (buffer == nullptr)
	{
		return 0;
	}

	*outAbsolutePathUtf8 = buffer;
	return 1;
}

int NN_ENGINE_ABI_STDCALL VfsWriteBufferToFile(
	const char* pathUtf8,
	const std::uint8_t* buffer,
	std::uint64_t size)
{
	if (pathUtf8 == nullptr || (buffer == nullptr && size > 0))
	{
		return 0;
	}

	return NN::Runtime::VFS::VFSService::WriteBufferToFile(
		pathUtf8,
		const_cast<std::uint8_t*>(buffer),
		size) ? 1 : 0;
}

std::uint64_t NN_ENGINE_ABI_STDCALL VfsAddFileSystem(
	const char* aliasUtf8,
	NNVfsFileSystemType type,
	const char* pathUtf8)
{
	if (aliasUtf8 == nullptr)
	{
		return 0;
	}
	// Memory 类型允许 path 为空
	if (type != NNVfsFS_Memory && pathUtf8 == nullptr)
	{
		return 0;
	}

	return NN::Runtime::VFS::VFSService::AddFileSystem(
		aliasUtf8,
		static_cast<std::uint32_t>(type),
		pathUtf8 ? pathUtf8 : "");
}

int NN_ENGINE_ABI_STDCALL VfsRemoveFileSystem(std::uint64_t handle)
{
	if (handle == 0)
	{
		return 0;
	}
	return NN::Runtime::VFS::VFSService::RemoveFileSystem(handle) ? 1 : 0;
}

int NN_ENGINE_ABI_STDCALL VfsHasFileSystem(std::uint64_t handle)
{
	if (handle == 0)
	{
		return 0;
	}
	return NN::Runtime::VFS::VFSService::HasFileSystem(handle) ? 1 : 0;
}

void NN_ENGINE_ABI_STDCALL VfsUnregisterAlias(const char* aliasUtf8)
{
	if (aliasUtf8 == nullptr)
	{
		return;
	}
	NN::Runtime::VFS::VFSService::UnregisterAlias(aliasUtf8);
}

int NN_ENGINE_ABI_STDCALL VfsIsAliasRegistered(const char* aliasUtf8)
{
	if (aliasUtf8 == nullptr)
	{
		return 0;
	}
	return NN::Runtime::VFS::VFSService::IsAliasRegistered(aliasUtf8) ? 1 : 0;
}

} // namespace

extern "C" void NNBuildVfsRuntimeApi(NNVfsAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	NNVfsAPI built{};
	built.size = static_cast<std::uint32_t>(sizeof(NNVfsAPI));
	built.readText = &VfsReadText;
	built.writeText = &VfsWriteText;
	built.readBytes = &VfsReadBytes;
	built.freeBuffer = &VfsFreeBuffer;
	built.getRelativePath = &VfsGetRelativePath;
	built.rebuildNativeFileSystemFiles = &VfsRebuildNativeFileSystemFiles;
	built.getAbsolutePath = &VfsGetAbsolutePath;
	built.writeBufferToFile = &VfsWriteBufferToFile;
	built.addFileSystem = &VfsAddFileSystem;
	built.removeFileSystem = &VfsRemoveFileSystem;
	built.hasFileSystem = &VfsHasFileSystem;
	built.unregisterAlias = &VfsUnregisterAlias;
	built.isAliasRegistered = &VfsIsAliasRegistered;
	*api = built;
}
