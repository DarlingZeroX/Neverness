/**
 * @file WindowApiStubs.cpp
 * @brief **NNWindowAPI** Stub：无 SDL；用于 Editor 测试与默认表。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "Engine/NativeInterop.h"
#include "Engine/WindowAPI.h"

namespace
{

NNWindowHandle NN_ENGINE_ABI_STDCALL stub_window_create(const NNWindowDesc* /*desc*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN_INVALID_WINDOW_HANDLE;
}

void NN_ENGINE_ABI_STDCALL stub_window_destroy(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_setTitle(NNWindowHandle /*handle*/, const char* /*title*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_setSize(NNWindowHandle /*handle*/, int /*width*/, int /*height*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_getSize(NNWindowHandle /*handle*/, int* /*outWidth*/, int* /*outHeight*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_setPosition(NNWindowHandle /*handle*/, int /*x*/, int /*y*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_getPosition(NNWindowHandle /*handle*/, int* /*outX*/, int* /*outY*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_setResizable(NNWindowHandle /*handle*/, bool /*value*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_maximize(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_minimize(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_restore(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_show(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_window_hide(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
}

void* NN_ENGINE_ABI_STDCALL stub_window_getNativeHandle(NNWindowHandle /*handle*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return nullptr;
}

} // namespace

extern "C" void NNBuildWindowApiStubs(NNWindowAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	api->size = static_cast<std::uint32_t>(sizeof(NNWindowAPI));
	api->create = &stub_window_create;
	api->destroy = &stub_window_destroy;
	api->setTitle = &stub_window_setTitle;
	api->setSize = &stub_window_setSize;
	api->getSize = &stub_window_getSize;
	api->setPosition = &stub_window_setPosition;
	api->getPosition = &stub_window_getPosition;
	api->setResizable = &stub_window_setResizable;
	api->maximize = &stub_window_maximize;
	api->minimize = &stub_window_minimize;
	api->restore = &stub_window_restore;
	api->show = &stub_window_show;
	api->hide = &stub_window_hide;
	api->getNativeHandle = &stub_window_getNativeHandle;
}
