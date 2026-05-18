/**
 * @file ObjectRuntimeApi.cpp
 * @brief **NNObjectAPI** Runtime 轉發至 **NNEngineRuntime::Object()**。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

NNObjectHandle NN_ENGINE_ABI_STDCALL rt_object_createObject(const char* typeNameUtf8)
{
	return NNEngineRuntime::Instance().Object().CreateObject(typeNameUtf8);
}

void NN_ENGINE_ABI_STDCALL rt_object_destroyObject(NNObjectHandle object)
{
	NNEngineRuntime::Instance().Object().DestroyObject(object);
}

void NN_ENGINE_ABI_STDCALL rt_object_retainObject(NNObjectHandle object)
{
	NNEngineRuntime::Instance().Object().RetainObject(object);
}

void NN_ENGINE_ABI_STDCALL rt_object_releaseObject(NNObjectHandle object)
{
	NNEngineRuntime::Instance().Object().ReleaseObject(object);
}

std::uint32_t NN_ENGINE_ABI_STDCALL rt_object_getRefCount(NNObjectHandle object)
{
	return NNEngineRuntime::Instance().Object().GetRefCount(object);
}

int NN_ENGINE_ABI_STDCALL rt_object_isAlive(NNObjectHandle object)
{
	return NNEngineRuntime::Instance().Object().IsAlive(object);
}

int NN_ENGINE_ABI_STDCALL rt_object_getTypeName(NNObjectHandle object, char* outUtf8, std::size_t outCapacity)
{
	return NNEngineRuntime::Instance().Object().GetTypeName(object, outUtf8, outCapacity);
}
} // namespace

extern "C" void NNBuildObjectRuntimeApi(NNObjectAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->createObject = &rt_object_createObject;
	api->destroyObject = &rt_object_destroyObject;
	api->retainObject = &rt_object_retainObject;
	api->releaseObject = &rt_object_releaseObject;
	api->getRefCount = &rt_object_getRefCount;
	api->isAlive = &rt_object_isAlive;
	api->getTypeName = &rt_object_getTypeName;
}
