#pragma once

/**
 * @file ObjectAPI.h
 * @brief 託管 **VGObject** 與 Native 之生命週期橋接（Phase 5 layout v3）。
 *
 * `getTypeName`：寫入 `outUtf8`（容量 `outCapacity`），回傳寫入字節數（不含 NUL）；失敗回傳 -1。
 */

#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef NNObjectHandle(NN_ENGINE_ABI_STDCALL* NNObjectCreateFn)(const char* typeNameUtf8);
typedef void(NN_ENGINE_ABI_STDCALL* NNObjectDestroyFn)(NNObjectHandle object);
typedef void(NN_ENGINE_ABI_STDCALL* NNObjectRetainFn)(NNObjectHandle object);
typedef void(NN_ENGINE_ABI_STDCALL* NNObjectReleaseFn)(NNObjectHandle object);
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNObjectGetRefCountFn)(NNObjectHandle object);
typedef int(NN_ENGINE_ABI_STDCALL* NNObjectIsAliveFn)(NNObjectHandle object);
typedef int(NN_ENGINE_ABI_STDCALL* NNObjectGetTypeNameFn)(
	NNObjectHandle object,
	char* outUtf8,
	size_t outCapacity);

typedef struct NNObjectAPI
{
	NNObjectCreateFn createObject;
	NNObjectDestroyFn destroyObject;
	NNObjectRetainFn retainObject;
	NNObjectReleaseFn releaseObject;
	NNObjectGetRefCountFn getRefCount;
	NNObjectIsAliveFn isAlive;
	NNObjectGetTypeNameFn getTypeName;
} NNObjectAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
