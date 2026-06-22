/**
 * @file InputApiStubs.cpp
 * @brief **NNInputAPI** 預設 Stub：按鍵恆為未按下。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "Engine/NativeInterop.h"

namespace
{
int NN_ENGINE_ABI_STDCALL stub_input_isKeyPressed(int keyCode)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)keyCode;
	return 0;
}
} // namespace

extern "C" void NNBuildInputApiStubs(NNInputAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->isKeyPressed = &stub_input_isKeyPressed;
}
