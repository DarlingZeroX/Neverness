/**
 * @file VfsApiStubs.cpp
 * @brief **NNVfsAPI** Stub：无 VFS 挂载；用于测试与默认表。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/VfsAPI.h"

namespace
{

int NN_ENGINE_ABI_STDCALL stub_vfs_readText(const char* /*pathUtf8*/, char** outText)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outText != nullptr)
	{
		*outText = nullptr;
	}
	return 0;
}

int NN_ENGINE_ABI_STDCALL stub_vfs_writeText(const char* /*pathUtf8*/, const char* /*textUtf8*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

int NN_ENGINE_ABI_STDCALL stub_vfs_readBytes(
	const char* /*pathUtf8*/,
	std::uint8_t** outData,
	std::uint32_t* outSize)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outData != nullptr)
	{
		*outData = nullptr;
	}
	if (outSize != nullptr)
	{
		*outSize = 0;
	}
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_vfs_freeBuffer(void* /*buffer*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

int NN_ENGINE_ABI_STDCALL stub_vfs_getRelativePath(
	const char* /*relativePathUtf8*/,
	const char* /*absolutePathUtf8*/,
	char** outPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outPathUtf8 != nullptr)
	{
		*outPathUtf8 = nullptr;
	}
	return 0;
}

int NN_ENGINE_ABI_STDCALL stub_vfs_rebuildNativeFileSystemFiles(const char* /*pathUtf8*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

int NN_ENGINE_ABI_STDCALL stub_vfs_getAbsolutePath(const char* /*relativePathUtf8*/, char** outAbsolutePathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	if (outAbsolutePathUtf8 != nullptr)
	{
		*outAbsolutePathUtf8 = nullptr;
	}
	return 0;
}

int NN_ENGINE_ABI_STDCALL stub_vfs_writeBufferToFile(
	const char* /*pathUtf8*/,
	const std::uint8_t* /*buffer*/,
	std::uint64_t /*size*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

} // namespace

extern "C" void NNBuildVfsApiStubs(NNVfsAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	api->size = static_cast<std::uint32_t>(sizeof(NNVfsAPI));
	api->readText = &stub_vfs_readText;
	api->writeText = &stub_vfs_writeText;
	api->readBytes = &stub_vfs_readBytes;
	api->freeBuffer = &stub_vfs_freeBuffer;
	api->getRelativePath = &stub_vfs_getRelativePath;
	api->rebuildNativeFileSystemFiles = &stub_vfs_rebuildNativeFileSystemFiles;
	api->getAbsolutePath = &stub_vfs_getAbsolutePath;
	api->writeBufferToFile = &stub_vfs_writeBufferToFile;
}
