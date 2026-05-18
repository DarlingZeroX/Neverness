/**
 * @file ObjectApiStubs.cpp
 * @brief **NNObjectAPI** Stub 函數指標實作，狀態委託 **ObjectStubDatabase**。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"
#include "Object/ObjectStubDatabase.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
NNObjectHandle NN_ENGINE_ABI_STDCALL stub_object_createObject(const char* typeNameUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::Object::CreateObject(typeNameUtf8);
}

void NN_ENGINE_ABI_STDCALL stub_object_destroyObject(NNObjectHandle object)
{
	NN::StubRuntime::BumpInvokeCount();
	NN::StubRuntime::Object::DestroyObject(object);
}

void NN_ENGINE_ABI_STDCALL stub_object_retainObject(NNObjectHandle object)
{
	NN::StubRuntime::BumpInvokeCount();
	NN::StubRuntime::Object::RetainObject(object);
}

void NN_ENGINE_ABI_STDCALL stub_object_releaseObject(NNObjectHandle object)
{
	NN::StubRuntime::BumpInvokeCount();
	NN::StubRuntime::Object::ReleaseObject(object);
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_object_getRefCount(NNObjectHandle object)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::Object::GetRefCount(object);
}

int NN_ENGINE_ABI_STDCALL stub_object_isAlive(NNObjectHandle object)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::Object::IsAlive(object) ? 1 : 0;
}

int NN_ENGINE_ABI_STDCALL stub_object_getTypeName(NNObjectHandle object, char* outUtf8, std::size_t outCapacity)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN::StubRuntime::Object::GetTypeName(object, outUtf8, outCapacity);
}
} // namespace

extern "C" void NNBuildObjectApiStubs(NNObjectAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->createObject = &stub_object_createObject;
	api->destroyObject = &stub_object_destroyObject;
	api->retainObject = &stub_object_retainObject;
	api->releaseObject = &stub_object_releaseObject;
	api->getRefCount = &stub_object_getRefCount;
	api->isAlive = &stub_object_isAlive;
	api->getTypeName = &stub_object_getTypeName;
}
