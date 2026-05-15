#pragma once

/**
 * @file ObjectAPI.h
 * @brief 託管 **VGObject** 與 Native 之生命週期橋接（Phase 5 layout v3）。
 *
 * `getTypeName`：寫入 `outUtf8`（容量 `outCapacity`），回傳寫入字節數（不含 NUL）；失敗回傳 -1。
 */

#include "VGNativeEngineAPI/EngineHandles.h"
#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VGObjectHandle(VG_ENGINE_ABI_STDCALL* VGObjectCreateFn)(const char* typeNameUtf8);
typedef void(VG_ENGINE_ABI_STDCALL* VGObjectDestroyFn)(VGObjectHandle object);
typedef void(VG_ENGINE_ABI_STDCALL* VGObjectRetainFn)(VGObjectHandle object);
typedef void(VG_ENGINE_ABI_STDCALL* VGObjectReleaseFn)(VGObjectHandle object);
typedef std::uint32_t(VG_ENGINE_ABI_STDCALL* VGObjectGetRefCountFn)(VGObjectHandle object);
typedef int(VG_ENGINE_ABI_STDCALL* VGObjectIsAliveFn)(VGObjectHandle object);
typedef int(VG_ENGINE_ABI_STDCALL* VGObjectGetTypeNameFn)(
	VGObjectHandle object,
	char* outUtf8,
	std::size_t outCapacity);

typedef struct VGObjectAPI
{
	VGObjectCreateFn createObject;
	VGObjectDestroyFn destroyObject;
	VGObjectRetainFn retainObject;
	VGObjectReleaseFn releaseObject;
	VGObjectGetRefCountFn getRefCount;
	VGObjectIsAliveFn isAlive;
	VGObjectGetTypeNameFn getTypeName;
} VGObjectAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
