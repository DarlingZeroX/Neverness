#pragma once

/**
 * @file VfsAPI.h
 * @brief **NNVfsAPI**：虚拟文件系统 Engine Service C ABI（Phase 1：文本与二进制读取/写入）。
 *
 * **职责边界**
 * - 负责：经已挂载的 **VFSService** 读写 VFS 路径（UTF-8）。
 * - 不负责：Mount、异步 IO、`std::function` 回调、Gameplay 语义。
 *
 * **内存所有权（工业级 C ABI）**
 * - `readText` / `readBytes` / `getRelativePath` / `getAbsolutePath` 通过 `malloc` 分配缓冲区，调用方 **必须** 经 `freeBuffer` 释放。
 * - 禁止返回裸 `const char*`（生命周期不明确）。
 * - 禁止在 ABI 层暴露 `std::string`、`Ref<>`、`std::vector`。
 *
 * **扩展规则**
 * - 仅允许在 `NNVfsAPI` 结构体 **尾部追加** 函数指针；破坏性变更须递增 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION`。
 */

#include <cstdint>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 读取 UTF-8 文本文件全文。
 * @param pathUtf8 NUL 结尾 VFS 虚拟路径；nullptr 失败。
 * @param outText 成功时写入 `malloc` 分配的 NUL 结尾缓冲区；调用方须 `freeBuffer`。
 * @return 非 0 成功；0 失败（`outText` 置为 nullptr）。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsReadTextFn)(const char* pathUtf8, char** outText);

/**
 * @brief 写入 UTF-8 文本（覆盖写）。
 * @return 非 0 成功；0 失败。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsWriteTextFn)(const char* pathUtf8, const char* textUtf8);

/**
 * @brief 读取二进制文件全文。
 * @param outData 成功时 `malloc` 分配；调用方须 `freeBuffer`。
 * @param outSize 成功时写入字节数；失败时置 0。
 * @return 非 0 成功；0 失败。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsReadBytesFn)(
	const char* pathUtf8,
	std::uint8_t** outData,
	std::uint32_t* outSize);

/**
 * @brief 释放由 `readText` / `readBytes` / `getRelativePath` / `getAbsolutePath` 分配的缓冲区（`free`）；nullptr 为 no-op。
 */
typedef void(NN_ENGINE_ABI_STDCALL* NNVfsFreeBufferFn)(void* buffer);

/**
 * @brief 计算 VFS 相对路径（对应 **VFSService::GetRelativePathVFS**）。
 * @param relativePathUtf8 VFS 相对路径锚点。
 * @param absolutePathUtf8 磁盘绝对路径（将规范为 Unix 风格后参与计算）。
 * @param outPathUtf8 成功时 `malloc` 分配的 NUL 结尾结果；须 `freeBuffer`。
 * @return 非 0 成功；0 失败。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsGetRelativePathFn)(
	const char* relativePathUtf8,
	const char* absolutePathUtf8,
	char** outPathUtf8);

/**
 * @brief 刷新指定路径下单一 Native 文件系统的文件列表（对应 **VFSService::RebuildNativeFileSystemFiles**）。
 * @param pathUtf8 VFS 路径前缀。
 * @return 非 0 成功；0 失败（非唯一 Native FS 等）。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsRebuildNativeFileSystemFilesFn)(const char* pathUtf8);

/**
 * @brief 将 VFS 虚拟路径解析为绝对路径（对应 **VFSService::GetAbsolutePath**）。
 * @param relativePathUtf8 VFS 虚拟路径。
 * @param outAbsolutePathUtf8 成功时 `malloc` 分配的 NUL 结尾结果；须 `freeBuffer`。
 * @return 非 0 成功；0 失败。
 */
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsGetAbsolutePathFn)(
	const char* relativePathUtf8,
	char** outAbsolutePathUtf8);

typedef struct NNVfsAPI
{
	std::uint32_t size;
	NNVfsReadTextFn readText;
	NNVfsWriteTextFn writeText;
	NNVfsReadBytesFn readBytes;
	NNVfsFreeBufferFn freeBuffer;
	NNVfsGetRelativePathFn getRelativePath;
	NNVfsRebuildNativeFileSystemFilesFn rebuildNativeFileSystemFiles;
	NNVfsGetAbsolutePathFn getAbsolutePath;
} NNVfsAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
